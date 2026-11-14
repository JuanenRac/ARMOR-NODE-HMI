# ARMOR-NODE-HMI screen simulator: main/ui.cpp (the real screen, LVGL 9) built for a computer with stand-ins for everything it talks to, and a program that writes one picture per
# language and scene. Built only by the workflow hmi-screens.yml; the firmware build does not know this folder. Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
cmake_minimum_required(VERSION 3.16)
project(armor_hmi_screen_sim C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

include(FetchContent)
set(LV_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h" CACHE STRING "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
# the version of the firmware (dependencies.lock)
FetchContent_Declare(lvgl GIT_REPOSITORY https://github.com/lvgl/lvgl.git GIT_TAG v9.3.0 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(lvgl)

set(MAIN "${CMAKE_CURRENT_SOURCE_DIR}/../main")
add_executable(armor_hmi_screen_sim
  main.cpp stubs.cpp "${MAIN}/ui.cpp"
  "${MAIN}/fonts/hmi_font_latin_s.c" "${MAIN}/fonts/hmi_font_latin_m.c" "${MAIN}/fonts/hmi_font_latin_l.c"
  "${MAIN}/fonts/hmi_font_ja_s.c" "${MAIN}/fonts/hmi_font_ja_m.c" "${MAIN}/fonts/hmi_font_ja_l.c"
  "${MAIN}/fonts/hmi_font_zh_s.c" "${MAIN}/fonts/hmi_font_zh_m.c" "${MAIN}/fonts/hmi_font_zh_l.c")
# the shim comes first: esp_log.h and esp_timer.h are the simulator's own
target_include_directories(armor_hmi_screen_sim BEFORE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/shim")
target_include_directories(armor_hmi_screen_sim PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${MAIN}" "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_compile_definitions(armor_hmi_screen_sim PRIVATE LV_CONF_PATH="${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h")
target_link_libraries(armor_hmi_screen_sim PRIVATE lvgl m "-Wl,--wrap=time")
