// ARMOR-NODE-HMI host double of ESP-IDF - esp_log.h: the log goes nowhere (the tests read what the code did, not what it said). Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#define ESP_LOGE(tag, fmt, ...) ((void)(tag))
#define ESP_LOGW(tag, fmt, ...) ((void)(tag))
#define ESP_LOGI(tag, fmt, ...) ((void)(tag))
#define ESP_LOGD(tag, fmt, ...) ((void)(tag))
