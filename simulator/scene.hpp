// ARMOR-NODE-HMI screen simulator - what the screen is told about the world while a picture is taken. Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <cstdint>

#include "node_control.hpp"
#include "power_watch.hpp"
#include "voice.hpp"
#include "core/server_view.hpp"
#include "core/view_model.hpp"

namespace sim {

struct Scene {
  armor::hmi::Link link = armor::hmi::Link::kNotConfigured;
  bool have_summary = false;
  bool current = false;
  armor::hmi::Summary summary;
  armor::hmi::ViewBoard board;                // what view_changed() hands to the screen
  armor::node_control::Status control;
  armor::power_watch::View power;
  armor::voice_task::View voice;
};

extern Scene g_scene;
extern std::int64_t g_now_us;   // the clock of esp_timer_get_time(), in microseconds

// Publishes the scene to the screen as a new block of the server's news.
inline void publish() {
  g_scene.board.publish(armor::hmi::make_view(g_scene.summary, g_scene.link, g_scene.have_summary, g_scene.current, false, ""));
}

}  // namespace sim
