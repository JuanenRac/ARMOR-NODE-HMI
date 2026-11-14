// ARMOR-NODE-HMI host double of ESP-IDF - esp_timer.h: a timer that the double of the MQTT client can fire by hand. Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <cstdint>

#include "esp_err.h"

struct host_timer;
typedef host_timer* esp_timer_handle_t;
typedef void (*esp_timer_cb_t)(void* argument);
struct esp_timer_create_args_t {
  esp_timer_cb_t callback = nullptr;
  void* arg = nullptr;
  const char* name = nullptr;
};
#ifdef __cplusplus
extern "C" {
#endif
esp_err_t esp_timer_create(const esp_timer_create_args_t* args, esp_timer_handle_t* out);
esp_err_t esp_timer_start_once(esp_timer_handle_t timer, std::uint64_t microseconds);
std::int64_t esp_timer_get_time(void);
#ifdef __cplusplus
}
#endif
