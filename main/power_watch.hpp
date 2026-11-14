// ARMOR-NODE-HMI - the battery of the panel: reads the gauge chip (BQ27220) every few seconds and tells what the readings mean (core/power_gauge.hpp: mains lost, battery low, gauge lost).
// Off by default (setting power.enabled). Nothing of it has run on a board: the register numbers are those of the chip's data sheet and the readings are tried on a computer with a stand-in chip.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include "core/hmi_config.hpp"

namespace armor::power_watch {

// What the screen shows of the battery.
struct View {
  bool have = false;          // a reading has come
  bool gauge_lost = false;    // the chip stopped answering
  bool on_battery = false;
  bool low = false;
  int percent = 0;
};

// Adds the chip to the I2C bus and starts the task, when the settings say so; false when it is off or the chip could not be added.
bool start(const config::Settings& settings);

View view();

}  // namespace armor::power_watch
