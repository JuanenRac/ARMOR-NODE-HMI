// ARMOR-NODE-HMI screen simulator - stands in for the timer of ESP-IDF: the clock is the one the simulator moves.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <cstdint>
extern "C" std::int64_t esp_timer_get_time(void);   // microseconds since the simulation started
