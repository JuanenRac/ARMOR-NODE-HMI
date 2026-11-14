// ARMOR-NODE-HMI host double of ESP-IDF - freertos/FreeRTOS.h: tasks are threads and a tick lasts ten microseconds, so the waits of the firmware take no time. Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <cstdint>

typedef void (*TaskFunction_t)(void*);
typedef void* TaskHandle_t;
typedef unsigned UBaseType_t;
typedef std::uint32_t TickType_t;
#define pdMS_TO_TICKS(ms) (static_cast<TickType_t>(ms))
#define pdTRUE 1
#define pdFALSE 0
