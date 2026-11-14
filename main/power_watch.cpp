// ARMOR-NODE-HMI - the battery of the panel (see power_watch.hpp). The rules are in core/power_gauge.hpp; this file is the I2C reading and the task.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include "power_watch.hpp"

#include <cstdio>
#include <mutex>
#include <vector>

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "core/board_s3.hpp"
#include "core/power_gauge.hpp"
#include "board_io.hpp"
#include "event_store.hpp"
#include "psram_mem.hpp"

namespace armor::power_watch {
namespace {
constexpr char kTag[] = "armor-power";
constexpr int kReadEveryMs = 5000;

std::mutex g_lock;
View g_view;
i2c_master_dev_handle_t g_gauge = nullptr;
hmi::PowerWatch g_watch;

// The chip's registers are 16 bits, little endian.
class I2cGauge final : public hmi::GaugeBus {
 public:
  bool read16(std::uint8_t reg, std::uint16_t& value) override {
    std::uint8_t bytes[2] = {0, 0};
    if (i2c_master_transmit_receive(g_gauge, &reg, 1, bytes, sizeof bytes, 100) != ESP_OK) return false;
    value = static_cast<std::uint16_t>(bytes[0] | (bytes[1] << 8));
    return true;
  }
};

void say(hmi::PowerEvent event, const hmi::Battery& battery) {
  char detail[48];
  std::snprintf(detail, sizeof detail, "charge %d %%, %d mV", battery.percent, battery.millivolts);
  ESP_LOGW(kTag, "%s (%s)", hmi::power_event_code(event), detail);
  event_store::add(hmi::power_event_code(event), detail);
}

void watch_task(void*) {
  I2cGauge gauge;
  std::vector<hmi::PowerEvent> events;
  for (;;) {
    const hmi::Battery reading = hmi::read_battery(gauge);
    events.clear();
    {
      std::lock_guard<std::mutex> guard(g_lock);
      g_watch.feed(reading, esp_timer_get_time() / 1000, events);
      g_view.have = g_watch.have_reading();
      g_view.gauge_lost = g_watch.gauge_lost();
      g_view.on_battery = g_watch.on_battery();
      g_view.low = g_watch.battery_low();
      g_view.percent = g_watch.last().percent;
    }
    for (hmi::PowerEvent event : events) say(event, reading);
    vTaskDelay(pdMS_TO_TICKS(kReadEveryMs));
  }
}
}  // namespace

bool start(const config::Settings& settings) {
  if (!settings.power.enabled || g_gauge != nullptr) return false;
  i2c_master_bus_handle_t bus = boardio::bus();
  if (bus == nullptr) { ESP_LOGW(kTag, "no I2C bus: the battery is not read"); return false; }
  i2c_device_config_t device{};
  device.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  device.device_address = hmi::bq27220::kAddress;
  device.scl_speed_hz = board::kI2cHz;
  if (i2c_master_bus_add_device(bus, &device, &g_gauge) != ESP_OK) { ESP_LOGW(kTag, "the gauge chip could not be added to the bus"); g_gauge = nullptr; return false; }
  g_watch.configure(settings.power.low_percent, settings.power.lost_confirm_s);
  memory::start_task(watch_task, "power-watch", 4096, nullptr, 2);   // it only reads the chip and writes a line: no flash of its own, so its stack is in the external RAM
  ESP_LOGI(kTag, "the battery is read every %d s (low at %d %%, mains lost after %d s on the battery)", kReadEveryMs / 1000, settings.power.low_percent, settings.power.lost_confirm_s);
  return true;
}

View view() {
  std::lock_guard<std::mutex> guard(g_lock);
  return g_view;
}

}  // namespace armor::power_watch
