// ARMOR-NODE-HMI screen simulator - the LVGL 9 configuration of the host build. Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
// Same colour depth as the firmware (sdkconfig.defaults: 16 bits) and the one built-in font the screen uses for its symbols; everything else is the default of LVGL.
#if 1
#ifndef LV_CONF_H
#define LV_CONF_H
#define LV_COLOR_DEPTH 16
#define LV_USE_STDLIB_MALLOC LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_STRING LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_SPRINTF LV_STDLIB_BUILTIN
#define LV_MEM_SIZE (8U * 1024U * 1024U)
#define LV_FONT_MONTSERRAT_14 1
#define LV_USE_LOG 0
#define LV_USE_OS LV_OS_NONE
#endif
#endif
