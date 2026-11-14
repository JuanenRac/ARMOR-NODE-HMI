// ARMOR-NODE-HMI - host tests of the battery gauge reading, what the panel makes of the readings over time, the rotating record of events and the settings that turn them on (all off by
// default), with a stand-in gauge chip and an in-memory stand-in for the files. No battery, no gauge chip and no storage were used.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <map>
#include <string>
#include <vector>

#include "../core/event_log.hpp"
#include "../core/hmi_config.hpp"
#include "../core/power_gauge.hpp"
#include "check.hpp"

using namespace armor;
using namespace armor::hmi;

// A stand-in for the gauge chip: registers it answers, and the ones it does not.
class FakeGauge : public GaugeBus {
 public:
  std::map<std::uint8_t, std::uint16_t> registers;
  std::vector<std::uint8_t> silent;
  bool read16(std::uint8_t reg, std::uint16_t& value) override {
    for (std::uint8_t s : silent) if (s == reg) return false;
    const auto found = registers.find(reg);
    if (found == registers.end()) return false;
    value = found->second;
    return true;
  }
  void set(int millivolts, int milliamps, int percent, int tenths_celsius = 250) {
    registers[bq27220::kVoltage] = static_cast<std::uint16_t>(millivolts);
    registers[bq27220::kCurrent] = static_cast<std::uint16_t>(static_cast<std::int16_t>(milliamps));
    registers[bq27220::kStateOfCharge] = static_cast<std::uint16_t>(percent);
    registers[bq27220::kTemperature] = static_cast<std::uint16_t>(tenths_celsius + 2731);
  }
};

static void test_reading() {
  FakeGauge gauge;
  CHECK(!read_battery(gauge).valid);   // the chip answers nothing
  gauge.set(4050, -350, 87, 231);
  Battery b = read_battery(gauge);
  CHECK(b.valid && b.millivolts == 4050 && b.milliamps == -350 && b.percent == 87 && b.tenths_celsius == 231 && b.giving() && !b.charging());
  gauge.set(4150, 800, 99);
  b = read_battery(gauge);
  CHECK(b.valid && b.charging() && !b.giving() && b.milliamps == 800);
  gauge.set(4100, 5, 90);
  b = read_battery(gauge);
  CHECK(b.valid && !b.charging() && !b.giving());   // a current this close to zero is neither
  // a register that does not answer, or a number a lithium cell cannot have, makes the reading invalid
  for (std::uint8_t reg : {bq27220::kVoltage, bq27220::kCurrent, bq27220::kStateOfCharge, bq27220::kTemperature}) {
    gauge.set(4000, 0, 50);
    gauge.silent = {reg};
    CHECK(!read_battery(gauge).valid);
    gauge.silent.clear();
  }
  gauge.set(2000, 0, 50);
  CHECK(!read_battery(gauge).valid);
  gauge.set(5200, 0, 50);
  CHECK(!read_battery(gauge).valid);
  gauge.set(4000, 0, 101);
  CHECK(!read_battery(gauge).valid);
  CHECK(std::string(power_event_code(PowerEvent::kPowerLost)) == "hmi_power_lost" && std::string(power_event_code(PowerEvent::kPowerBack)) == "hmi_power_back");
}

static Battery reading(int milliamps, int percent) {
  Battery b;
  b.valid = true;
  b.millivolts = 3900;
  b.milliamps = milliamps;
  b.percent = percent;
  return b;
}

static void test_watch() {
  PowerWatch watch;
  watch.configure(20, 10);
  std::vector<PowerEvent> events;
  // on the mains: nothing happens
  for (int t = 0; t < 30; ++t) watch.feed(reading(300, 95), t * 1000, events);
  CHECK(events.empty() && !watch.on_battery() && watch.have_reading());
  // a dip of a few seconds is not a loss
  for (int t = 30; t < 36; ++t) watch.feed(reading(-400, 95), t * 1000, events);
  watch.feed(reading(300, 95), 36000, events);
  CHECK(events.empty() && !watch.on_battery());
  // the battery gives for ten seconds in a row: the mains are lost, once
  for (int t = 40; t <= 49; ++t) watch.feed(reading(-400, 94), t * 1000, events);
  CHECK(events.empty());
  watch.feed(reading(-400, 94), 50000, events);
  CHECK(events.size() == 1 && events[0] == PowerEvent::kPowerLost && watch.on_battery());
  for (int t = 51; t < 60; ++t) watch.feed(reading(-400, 93), t * 1000, events);
  CHECK(events.size() == 1);   // not repeated
  // the charge falls to the low limit: said once
  watch.feed(reading(-400, 20), 60000, events);
  CHECK(events.size() == 2 && events[1] == PowerEvent::kBatteryLow && watch.battery_low());
  watch.feed(reading(-400, 15), 61000, events);
  CHECK(events.size() == 2);
  // the mains come back: after three seconds, and the low state ends with them
  watch.feed(reading(500, 15), 62000, events);
  watch.feed(reading(500, 15), 63000, events);
  CHECK(events.size() == 2);
  watch.feed(reading(500, 16), 65000, events);
  CHECK(events.size() == 4 && events[2] == PowerEvent::kPowerBack && events[3] == PowerEvent::kBatteryOk && !watch.on_battery() && !watch.battery_low());
  // low is only said while on the battery, and clears five points higher
  events.clear();
  PowerWatch second;
  second.configure(30, 3);
  second.feed(reading(300, 10), 0, events);
  CHECK(events.empty());   // on the mains a low charge says nothing
  for (int t = 1; t <= 4; ++t) second.feed(reading(-300, 29), t * 1000, events);
  CHECK(events.size() == 2 && events[0] == PowerEvent::kPowerLost && events[1] == PowerEvent::kBatteryLow);
  second.feed(reading(-300, 34), 5000, events);
  CHECK(events.size() == 2);
  second.feed(reading(-300, 35), 6000, events);
  CHECK(events.size() == 3 && events[2] == PowerEvent::kBatteryOk && !second.battery_low());
}

static void test_gauge_lost() {
  PowerWatch watch;
  watch.configure(20, 10);
  std::vector<PowerEvent> events;
  const Battery nothing;
  for (int t = 0; t < 4; ++t) watch.feed(nothing, t * 1000, events);
  CHECK(events.empty() && !watch.have_reading());
  watch.feed(nothing, 4000, events);
  CHECK(events.size() == 1 && events[0] == PowerEvent::kGaugeLost && watch.gauge_lost());
  for (int t = 5; t < 12; ++t) watch.feed(nothing, t * 1000, events);
  CHECK(events.size() == 1);   // once
  watch.feed(reading(300, 80), 12000, events);
  CHECK(events.size() == 2 && events[1] == PowerEvent::kGaugeBack && !watch.gauge_lost() && watch.last().percent == 80);
  // a good reading in between restarts the count of silent ones
  PowerWatch other;
  other.configure(20, 10);
  events.clear();
  for (int t = 0; t < 4; ++t) other.feed(nothing, t * 1000, events);
  other.feed(reading(300, 80), 4000, events);
  for (int t = 5; t < 9; ++t) other.feed(nothing, t * 1000, events);
  CHECK(events.empty());
  // the limits are clamped
  PowerWatch clamp;
  clamp.configure(1, 1);
  clamp.feed(reading(-400, 5), 0, events);
  clamp.feed(reading(-400, 5), 3000, events);
  CHECK(events.size() == 2 && events[0] == PowerEvent::kPowerLost && events[1] == PowerEvent::kBatteryLow);   // 5 % is the lowest limit, 3 s the shortest wait
}

// A stand-in for the files: names to contents, with a size limit for the free space.
class MemoryFiles : public LogFiles {
 public:
  std::map<std::string, std::string> files;
  long free = 10L * 1024 * 1024;
  bool fail_append = false;
  bool append(const std::string& name, std::string_view data) override { if (fail_append) return false; files[name].append(data); return true; }
  long size(const std::string& name) override { const auto f = files.find(name); return f == files.end() ? -1 : static_cast<long>(f->second.size()); }
  bool rename(const std::string& from, const std::string& to) override { const auto f = files.find(from); if (f == files.end()) return false; files[to] = f->second; files.erase(from); return true; }
  bool remove(const std::string& name) override { return files.erase(name) > 0; }
  long free_bytes() override { return free; }
  bool read(const std::string& name, std::string& out, std::size_t max_bytes) override {
    const auto f = files.find(name);
    if (f == files.end()) return false;
    out = f->second.size() > max_bytes ? f->second.substr(f->second.size() - max_bytes) : f->second;
    return true;
  }
};

static void test_log_lines() {
  CHECK(EventLog::make_line(1790000000, "hmi_power_lost", "battery 94 %") == "1790000000 hmi_power_lost battery 94 %\n");
  CHECK(EventLog::make_line(-5, "", "") == "0 event \n");
  // one event is one line whatever it is made of
  const std::string line = EventLog::make_line(1, "a b\nc", "first\nsecond\r\tthird \xC3\xA9");
  CHECK(line == "1 a_b_c first?second??third ??\n");
  CHECK(line.find('\n') == line.size() - 1);
  const std::string longest = EventLog::make_line(1, std::string(100, 'x'), std::string(500, 'y'));
  CHECK(longest.size() <= kLogLineMax && longest.back() == '\n');
  CHECK(longest.find(std::string(33, 'x')) == std::string::npos);   // the code is cut to 32
}

static void test_log_rotation() {
  MemoryFiles files;
  EventLog log(files);
  log.configure(16, 3, 0);   // 16 KiB a file, three files
  CHECK(log.max_bytes() == 16 * 1024 && log.keep() == 3);
  CHECK(log.text(100000).empty());
  CHECK(log.add(1000, "hmi_power_lost", "first") && files.files.count("events.log") == 1 && files.files["events.log"] == "1000 hmi_power_lost first\n");
  // fill past the limit: the file rotates, never grows past the limit, and only `keep` files exist
  const std::string detail(100, 'd');
  for (int i = 0; i < 800; ++i) CHECK(log.add(2000 + i, "hmi_battery_low", detail));
  CHECK(files.files.count("events.log") == 1 && files.files.count("events.1") == 1 && files.files.count("events.2") == 1 && files.files.count("events.3") == 0);
  for (const auto& entry : files.files) CHECK(static_cast<long>(entry.second.size()) <= 16 * 1024);
  CHECK(log.written() == 801 && log.dropped() == 0 && log.failed() == 0);
  // the newest lines survive, the oldest are gone, and the text comes oldest first
  const std::string all = log.text(1000000);
  CHECK(all.find("\n2799 hmi_battery_low") != std::string::npos);
  CHECK(all.find("1000 hmi_power_lost first") == std::string::npos);
  const std::size_t a = all.find("\n2700 hmi_battery_low"), b = all.find("\n2799 hmi_battery_low");
  CHECK(a != std::string::npos && b != std::string::npos && a < b);
  // asking for less returns whole lines, the newest ones
  const std::string tail = log.text(500);
  CHECK(tail.size() <= 500 && !tail.empty() && tail.back() == '\n' && tail.find("2799 ") != std::string::npos && tail[0] == '2');
  // keeping only two files
  MemoryFiles two;
  EventLog small(two);
  small.configure(16, 2, 0);
  for (int i = 0; i < 400; ++i) small.add(i, "x", detail);
  CHECK(two.files.count("events.log") == 1 && two.files.count("events.1") == 1 && two.files.size() == 2);
}

static void test_log_guards() {
  MemoryFiles files;
  EventLog log(files);
  log.configure(256, 4, 64);
  files.free = 64 * 1024 + 10;   // no room for the line and the reserve
  CHECK(!log.add(1, "x", "a line of some length that does not fit the reserve") && log.dropped() == 1 && files.files.empty());
  files.free = 10L * 1024 * 1024;
  CHECK(log.add(2, "x", "fits") && log.written() == 1);
  files.fail_append = true;
  CHECK(!log.add(3, "x", "the write fails") && log.failed() == 1);
  // the limits are clamped
  log.configure(1, 1, -5);
  CHECK(log.max_bytes() == kLogMinKb * 1024L && log.keep() == kLogMinKeep);
  log.configure(100000, 100, 0);
  CHECK(log.max_bytes() == kLogMaxKb * 1024L && log.keep() == kLogMaxKeep);
}

static void test_settings() {
  config::Settings base = config::default_settings("a1b2c3");
  base.sta.enabled = true;
  base.sta.ssid = "casa";
  base.sta.password = "una-clave-larga";
  CHECK(!base.power.enabled && base.power.low_percent == 20 && base.power.lost_confirm_s == 10 && !base.storage.enabled && base.storage.max_kb == 256 && base.storage.keep == 4);
  config::Settings out;
  config::Problems problems;
  const char* document = "{\"power\":{\"enabled\":true,\"low_percent\":30,\"lost_confirm_s\":20},\"storage\":{\"enabled\":true,\"max_kb\":512,\"keep\":6}}";
  CHECK(config::load(document, base, out, problems) && problems.empty());
  CHECK(out.power.enabled && out.power.low_percent == 30 && out.power.lost_confirm_s == 20 && out.storage.enabled && out.storage.max_kb == 512 && out.storage.keep == 6);
  const std::string stored = config::to_json(out, true);
  CHECK(stored.find("\"power\":{\"enabled\":true,\"low_percent\":30,\"lost_confirm_s\":20}") != std::string::npos);
  CHECK(stored.find("\"storage\":{\"enabled\":true,\"max_kb\":512,\"keep\":6}") != std::string::npos);
  config::Settings back;
  problems.clear();
  CHECK(config::load(stored, base, back, problems) && back.power.low_percent == 30 && back.storage.keep == 6 && config::to_json(back, true) == stored);
  CHECK(config::to_json(base, true).find("\"power\":{\"enabled\":false,\"low_percent\":20,\"lost_confirm_s\":10}") != std::string::npos);
  const auto refuses = [&](const char* text, const char* path) {
    config::Problems p;
    config::Settings o;
    if (config::load(text, base, o, p)) return false;
    for (const config::Problem& item : p) if (item.path == path) return true;
    return false;
  };
  CHECK(refuses("{\"power\":{\"enabled\":\"yes\"}}", "power.enabled"));
  CHECK(refuses("{\"power\":{\"low_percent\":4}}", "power.low_percent"));
  CHECK(refuses("{\"power\":{\"low_percent\":51}}", "power.low_percent"));
  CHECK(refuses("{\"power\":{\"lost_confirm_s\":2}}", "power.lost_confirm_s"));
  CHECK(refuses("{\"power\":{\"lost_confirm_s\":121}}", "power.lost_confirm_s"));
  CHECK(refuses("{\"storage\":{\"enabled\":1}}", "storage.enabled"));
  CHECK(refuses("{\"storage\":{\"max_kb\":15}}", "storage.max_kb"));
  CHECK(refuses("{\"storage\":{\"max_kb\":4097}}", "storage.max_kb"));
  CHECK(refuses("{\"storage\":{\"keep\":1}}", "storage.keep"));
  CHECK(refuses("{\"storage\":{\"keep\":11}}", "storage.keep"));
}

int main() {
  test_reading();
  test_watch();
  test_gauge_lost();
  test_log_lines();
  test_log_rotation();
  test_log_guards();
  test_settings();
  FINISH("power_storage");
}
