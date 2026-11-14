// ARMOR-NODE-HMI - the panel's own record of events in the flash partition "storage" (see core/event_log.hpp: what a line is, how the files rotate and when nothing is written).
// Off by default (setting storage.enabled). The partition is mounted only when it is on. The microSD socket of the board is not used yet: its chip select is a line of the I/O expander, which the
// card driver of ESP-IDF cannot drive, so a card needs its own work on a real board.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <cstddef>
#include <string>
#include <string_view>

#include "core/hmi_config.hpp"

namespace armor::event_store {

// Mounts the partition and starts the record when the settings say so; false when it is off or the partition could not be mounted.
bool start(const config::Settings& settings);

// True while the record is running.
bool active();

// Writes one event (the code of core/power_gauge.hpp, or any other short word) with its detail. Does nothing while the record is not running.
void add(std::string_view code, std::string_view detail);

// The record as text, oldest line first, at most `max_bytes` of the newest.
std::string text(std::size_t max_bytes);

}  // namespace armor::event_store
