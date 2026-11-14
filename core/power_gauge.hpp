// ARMOR-NODE-HMI - the battery of the panel: the reading of its gauge chip (BQ27220) and what the panel makes of the readings over time (mains lost, battery low). Pure: the chip is reached
// through a small interface, so all of it is tested on a computer with a stand-in chip. Nothing of it has run on a board.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// The BQ27220 answers at 0x55 on the shared I2C bus with 16-bit little-endian registers ("standard commands"): the temperature in tenths of a kelvin (0x06), the voltage in millivolts (0x08),
// the current in milliamps, signed, negative while the battery gives (0x0C) and the state of charge in percent (0x2C). A reading is valid only when the chip answered every register and the
// numbers are possible for a lithium cell.
//
// What the panel decides from a run of readings (setting power.enabled, off by default):
//   * mains lost: the battery has been giving for lost_confirm_s seconds in a row (a short dip is not a loss);
//   * mains back: the battery has not been giving for 3 seconds in a row;
//   * battery low: on battery and at or below low_percent; it clears at 5 points higher or when the mains are back;
//   * gauge lost / back: the chip stopped answering for 5 readings in a row, and answered again.
// Each of these is said once, as an event, and never repeated until its opposite has happened.
#pragma once
#include <cstdint>
#include <vector>

namespace armor::hmi {

class GaugeBus {
 public:
  virtual ~GaugeBus() = default;
  // Reads one 16-bit register of the gauge; false when the chip does not answer.
  virtual bool read16(std::uint8_t reg, std::uint16_t& value) = 0;
};

namespace bq27220 {
constexpr std::uint8_t kAddress = 0x55;
constexpr std::uint8_t kTemperature = 0x06, kVoltage = 0x08, kCurrent = 0x0C, kStateOfCharge = 0x2C;
constexpr int kIdleMilliamps = 20;   // a current this close to zero is neither charging nor giving
}  // namespace bq27220

struct Battery {
  bool valid = false;
  int millivolts = 0, milliamps = 0, percent = 0, tenths_celsius = 0;
  bool giving() const { return valid && milliamps < -bq27220::kIdleMilliamps; }
  bool charging() const { return valid && milliamps > bq27220::kIdleMilliamps; }
};

// One reading of the chip. `valid` is false when a register did not answer or a number is not possible (the voltage of a lithium cell is 2.5 to 5 V, the charge 0 to 100 %).
inline Battery read_battery(GaugeBus& bus) {
  Battery b;
  std::uint16_t temperature = 0, voltage = 0, current = 0, charge = 0;
  if (!bus.read16(bq27220::kVoltage, voltage) || !bus.read16(bq27220::kCurrent, current) || !bus.read16(bq27220::kStateOfCharge, charge) || !bus.read16(bq27220::kTemperature, temperature)) return b;
  if (voltage < 2500 || voltage > 5000 || charge > 100) return b;
  b.millivolts = voltage;
  b.milliamps = static_cast<std::int16_t>(current);
  b.percent = charge;
  b.tenths_celsius = static_cast<int>(temperature) - 2731;   // tenths of a kelvin to tenths of a degree
  b.valid = true;
  return b;
}

enum class PowerEvent { kPowerLost, kPowerBack, kBatteryLow, kBatteryOk, kGaugeLost, kGaugeBack };

inline const char* power_event_code(PowerEvent event) {
  switch (event) {
    case PowerEvent::kPowerLost: return "hmi_power_lost";
    case PowerEvent::kPowerBack: return "hmi_power_back";
    case PowerEvent::kBatteryLow: return "hmi_battery_low";
    case PowerEvent::kBatteryOk: return "hmi_battery_ok";
    case PowerEvent::kGaugeLost: return "hmi_gauge_lost";
    case PowerEvent::kGaugeBack: return "hmi_gauge_back";
  }
  return "hmi_power";
}

struct PowerSettings {
  bool enabled = false;
  int low_percent = 20;       // 5 to 50
  int lost_confirm_s = 10;    // 3 to 120
};

class PowerWatch {
 public:
  void configure(int low_percent, int lost_confirm_s) {
    low_percent_ = low_percent < 5 ? 5 : (low_percent > 50 ? 50 : low_percent);
    lost_confirm_ms_ = static_cast<std::int64_t>(lost_confirm_s < 3 ? 3 : (lost_confirm_s > 120 ? 120 : lost_confirm_s)) * 1000;
  }

  // One reading; the events it brings are added to `events`.
  void feed(const Battery& reading, std::int64_t now_ms, std::vector<PowerEvent>& events) {
    if (!reading.valid) {
      if (++bad_run_ == kGaugeBadReadings && !gauge_lost_) { gauge_lost_ = true; events.push_back(PowerEvent::kGaugeLost); }
      return;
    }
    bad_run_ = 0;
    if (gauge_lost_) { gauge_lost_ = false; events.push_back(PowerEvent::kGaugeBack); }
    last_ = reading;
    have_ = true;
    if (reading.giving()) {
      if (giving_since_ < 0) giving_since_ = now_ms;
      steady_since_ = -1;
      if (!on_battery_ && now_ms - giving_since_ >= lost_confirm_ms_) { on_battery_ = true; events.push_back(PowerEvent::kPowerLost); }
    } else {
      giving_since_ = -1;
      if (steady_since_ < 0) steady_since_ = now_ms;
      if (on_battery_ && now_ms - steady_since_ >= kBackConfirmMs) {
        on_battery_ = false;
        events.push_back(PowerEvent::kPowerBack);
        if (low_) { low_ = false; events.push_back(PowerEvent::kBatteryOk); }
      }
    }
    if (on_battery_ && !low_ && reading.percent <= low_percent_) { low_ = true; events.push_back(PowerEvent::kBatteryLow); }
    else if (low_ && reading.percent >= low_percent_ + 5) { low_ = false; events.push_back(PowerEvent::kBatteryOk); }
  }

  bool have_reading() const { return have_; }
  const Battery& last() const { return last_; }
  bool on_battery() const { return on_battery_; }
  bool battery_low() const { return low_; }
  bool gauge_lost() const { return gauge_lost_; }

 private:
  static constexpr int kGaugeBadReadings = 5;
  static constexpr std::int64_t kBackConfirmMs = 3000;
  int low_percent_ = 20;
  std::int64_t lost_confirm_ms_ = 10000;
  Battery last_;
  bool have_ = false, on_battery_ = false, low_ = false, gauge_lost_ = false;
  int bad_run_ = 0;
  std::int64_t giving_since_ = -1, steady_since_ = -1;
};

}  // namespace armor::hmi
