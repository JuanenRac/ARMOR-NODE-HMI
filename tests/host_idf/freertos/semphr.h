// ARMOR-NODE-HMI host double of ESP-IDF - freertos/semphr.h. Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include "FreeRTOS.h"

struct host_semaphore;
typedef host_semaphore* SemaphoreHandle_t;
SemaphoreHandle_t xSemaphoreCreateBinary();
int xSemaphoreTake(SemaphoreHandle_t semaphore, TickType_t ticks);
int xSemaphoreGive(SemaphoreHandle_t semaphore);
