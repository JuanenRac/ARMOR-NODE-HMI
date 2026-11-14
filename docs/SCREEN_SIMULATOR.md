# The screen drawn on a computer

`simulator/` builds `main/ui.cpp` (the real screen of the panel, LVGL 9, 800x480) for a computer and draws it in the eight languages. **Evidence: simulator.** The pictures show what the code draws with a stand-in server; nothing here has run on the panel.

## What is real and what is a stand-in

- **Real, unchanged:** `main/ui.cpp`, the nine fonts of `main/fonts` (Inter, Noto Sans JP and Noto Sans SC), the words of `core/screen_text.hpp`, the view model, the countdown, the PIN entry and the rest of `core/`. LVGL is the version of the firmware (9.3.0, `dependencies.lock`) with the same colour depth (16 bits).
- **In memory:** the link to the server (`server_link`), the node control, the battery gauge, the voice assistant, the speaker and the display driver (`simulator/stubs.cpp`), and the logging and timer headers of ESP-IDF (`simulator/shim`). The scene being drawn (`simulator/scene.hpp`) says what each of them answers.
- **Not covered:** the touch panel and the display of the board, the backlight, the speaker, the real server and the real network. A picture says nothing about them.

## The pictures

Six scenes in each of the eight languages (48 pictures, `images/screens/<language>/<scene>.png`, 128 colours, at most 400 KiB each):

| Scene | What it shows |
|---|---|
| `setup` | a panel with no user yet: the screen only says how to set it up |
| `disarmed` | the system disarmed, two alarms, the nodes, the solar equipment and the battery |
| `confirm` | the question the screen asks before arming (a touch on the button, in the simulator) |
| `armed` | the system armed and quiet |
| `alarm` | armed, two critical alarms and a warning |
| `paused` | the panel paused by the server |

## Making them again

The simulator is built and run only in GitHub Actions, by the workflow `hmi-screens.yml` (started by hand; it builds LVGL, so it is not part of the normal checks): it builds `simulator/`, draws the 48 pictures, turns them into PNG files (`simulator/ppm_to_png.py`, needs Pillow) and keeps them as an artifact for two weeks. Download the artifact into `images/screens/` to renew the pictures. The clock of the pictures is fixed (09:41), so two runs of the same code give the same pictures.

On a computer with CMake, a C++17 compiler and network access to fetch LVGL: `cmake -S simulator -B build-sim && cmake --build build-sim && build-sim/armor_hmi_screen_sim shots`, then `python simulator/ppm_to_png.py shots images/screens`.

## Adding a scene

Put what the screen is told in `simulator/main.cpp` (`online(...)` for the summary of the server, `sim::g_scene.control` for a paused panel, `touch(x, y)` for a finger), take it with `save(directory, language, "name")` and add the name to the list above and to the check of the workflow (`ls shots/*.ppm | wc -l`).
