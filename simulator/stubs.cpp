// ARMOR-NODE-HMI screen simulator - the parts of the firmware that main/ui.cpp talks to, in memory. Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// main/ui.cpp is compiled unchanged. It asks the link to the server for a summary, the node control for the state of the panel, the battery gauge and the voice assistant for
// their views, and the display for a lock; here each of them answers what the scene being drawn says (scene.hpp). Nothing here opens a socket or a file.
#include "scene.hpp"

#include <ctime>

#include "audio.hpp"
#include "display.hpp"
#include "node_control.hpp"
#include "power_watch.hpp"
#include "server_link.hpp"
#include "voice.hpp"

namespace sim {
Scene g_scene;
std::int64_t g_now_us = 0;
}  // namespace sim

extern "C" std::int64_t esp_timer_get_time(void) { return sim::g_now_us; }

// ui.cpp shows the time of day: the simulator's clock is fixed (--wrap=time), so the pictures are the same on every run.
extern "C" std::time_t __wrap_time(std::time_t* out) {
  constexpr std::time_t kFixed = 1'790'000'000 + 9 * 3600 + 41 * 60;   // 09:41 on some day
  if (out != nullptr) *out = kFixed;
  return kFixed;
}

namespace armor {
namespace audio {
bool ready() { return false; }
void beep(Tone) {}
}  // namespace audio

namespace display {
bool lock(int) { return true; }
void unlock() {}
bool asleep() { return false; }
lv_display_t* handle() { return lv_display_get_default(); }
}  // namespace display

namespace node_control {
Status status() { return sim::g_scene.control; }
}  // namespace node_control

namespace power_watch {
View view() { return sim::g_scene.power; }
}  // namespace power_watch

namespace voice_task {
void press() {}
View view() { return sim::g_scene.voice; }
}  // namespace voice_task

namespace server_link {
Snapshot snapshot() {
  Snapshot snap;
  snap.link = sim::g_scene.link;
  snap.have_summary = sim::g_scene.have_summary;
  snap.current = sim::g_scene.current;
  snap.summary = sim::g_scene.summary;
  return snap;
}
bool view_changed(std::uint32_t& seen, hmi::ViewModel& out) { return sim::g_scene.board.changed_since(seen, out); }
void request_mode(hmi::Mode, const std::string&) {}
void request_acknowledge_all(const std::string&) {}
void request_preflight() {}
PreflightState preflight_state() { return {}; }
void camera_open(const std::string&, int) {}
void camera_close() {}
bool camera_take(std::uint32_t&, std::string&, std::uint32_t&, std::uint32_t&, std::string&) { return false; }
}  // namespace server_link
}  // namespace armor
