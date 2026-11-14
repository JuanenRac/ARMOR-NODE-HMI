// ARMOR-NODE-HMI host double of ESP-IDF - freertos/task.h. Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include "FreeRTOS.h"

int xTaskCreate(TaskFunction_t function, const char* name, std::uint32_t stack, void* argument, UBaseType_t priority, TaskHandle_t* handle);
void vTaskDelay(TickType_t ticks);
void vTaskDelete(TaskHandle_t task);
