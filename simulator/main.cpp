// ARMOR-NODE-HMI screen simulator - draws the real screen (main/ui.cpp, LVGL 9, 800x480) on a computer and writes one picture per language and scene as a PPM file.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
//   armor_hmi_screen_sim OUTPUT_DIR [language ...]       writes OUTPUT_DIR/<language>_<scene>.ppm (all eight languages when none is named)
//
// Evidence: simulator. The pictures show what the code draws with a stand-in server; nothing here has run on the panel. The screen is the one of the firmware: the same source file, the
// same fonts, the same words (core/screen_text.hpp); only the link to the server, the battery gauge, the voice assistant and the display driver are in memory (stubs.cpp).
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "lvgl.h"
#include "scene.hpp"
#include "ui.hpp"

namespace {
constexpr int kWidth = 800, kHeight = 480;
std::uint16_t g_buffer[kWidth * kHeight];   // what LVGL draws into
std::uint16_t g_frame[kWidth * kHeight];    // the last full picture
std::uint32_t g_tick_ms = 0;
int g_px = 0, g_py = 0;
bool g_pressed = false;
armor::ui::Context g_context;

std::uint32_t tick_cb() { return g_tick_ms; }

void flush_cb(lv_display_t* display, const lv_area_t*, std::uint8_t* pixels) {
  std::memcpy(g_frame, pixels, sizeof g_frame);
  lv_display_flush_ready(display);
}

void pointer_cb(lv_indev_t*, lv_indev_data_t* data) {
  data->point.x = g_px;
  data->point.y = g_py;
  data->state = g_pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

void advance(int ms) {
  for (int spent = 0; spent < ms; spent += 10) {
    g_tick_ms += 10;
    sim::g_now_us += 10'000;
    lv_timer_handler();
  }
}

void touch(int x, int y) {
  g_px = x;
  g_py = y;
  g_pressed = true;
  advance(120);
  g_pressed = false;
  advance(120);
}

bool save(const std::string& directory, const std::string& language, const char* scene) {
  lv_refr_now(nullptr);
  const std::string path = directory + "/" + language + "_" + scene + ".ppm";
  std::FILE* file = std::fopen(path.c_str(), "wb");
  if (file == nullptr) {
    std::fprintf(stderr, "cannot write %s\n", path.c_str());
    return false;
  }
  std::fprintf(file, "P6\n%d %d\n255\n", kWidth, kHeight);
  std::vector<std::uint8_t> row(kWidth * 3);
  for (int y = 0; y < kHeight; ++y) {
    for (int x = 0; x < kWidth; ++x) {
      const std::uint16_t p = g_frame[y * kWidth + x];
      row[x * 3 + 0] = static_cast<std::uint8_t>(((p >> 11) & 0x1F) * 255 / 31);
      row[x * 3 + 1] = static_cast<std::uint8_t>(((p >> 5) & 0x3F) * 255 / 63);
      row[x * 3 + 2] = static_cast<std::uint8_t>((p & 0x1F) * 255 / 31);
    }
    std::fwrite(row.data(), 1, row.size(), file);
  }
  std::fclose(file);
  return true;
}

armor::hmi::AlarmLine alarm(const char* id, const char* code, armor::hmi::Severity severity) {
  armor::hmi::AlarmLine line;
  line.id = id;
  line.code = code;
  line.severity = severity;
  line.source_type = "node";
  line.source_id = "radar-1";
  line.raised_at = "2026-01-01T09:30:00Z";
  return line;
}

void online(armor::hmi::Mode mode, int nodes_online, int nodes_total, std::vector<armor::hmi::AlarmLine> alarms) {
  using namespace armor::hmi;
  sim::Scene& scene = sim::g_scene;
  scene.link = Link::kOnline;
  scene.have_summary = true;
  scene.current = true;
  scene.summary = Summary{};
  scene.summary.mode = mode;
  scene.summary.nodes_online = nodes_online;
  scene.summary.nodes_total = nodes_total;
  scene.summary.alarms = std::move(alarms);
  scene.summary.alarms_active = static_cast<int>(scene.summary.alarms.size());
  scene.summary.alarms_unacknowledged = scene.summary.alarms_active;
  scene.summary.energy.present = true;
  scene.summary.energy.reporting = true;
  scene.summary.energy.pv_w = 2400;
  scene.summary.energy.load_w = 650;
  scene.summary.energy.has_battery = true;
  scene.summary.energy.battery_w = 900;
  scene.summary.energy.soc_percent = 78;
  sim::publish();
}

void scene_disarmed() {
  using namespace armor::hmi;
  online(Mode::kDisarmed, 4, 5, {alarm("a1", "door_open", Severity::kWarning), alarm("a2", "node_down", Severity::kHigh)});
}
void scene_armed() { online(armor::hmi::Mode::kArmed, 5, 5, {}); }
void scene_alarm() {
  using namespace armor::hmi;
  online(Mode::kArmed, 5, 5, {alarm("b1", "intrusion", Severity::kCritical), alarm("b2", "smoke", Severity::kCritical), alarm("b3", "camera_down", Severity::kWarning)});
}

bool take_language(const std::string& directory, const std::string& language) {
  g_context = armor::ui::Context{};
  g_context.language = language;
  g_context.version = "0.0.0";
  g_context.ip = "192.168.1.50";
  g_context.voice_enabled = true;
  g_context.wake_name = "armor";
  const armor::hmi::ViewBoard board = sim::g_scene.board;   // the generation of the news goes on counting from one language to the next
  sim::g_scene = sim::Scene{};
  sim::g_scene.board = board;
  bool ok = true;

  // a panel that has no user yet: the screen only says how to set it up
  g_context.setup = true;
  sim::publish();
  advance(1200);
  ok = save(directory, language, "setup") && ok;
  g_context.setup = false;

  scene_disarmed();
  advance(1200);
  ok = save(directory, language, "disarmed") && ok;

  touch(200, 150);   // the button to arm: the screen asks first
  advance(300);
  ok = save(directory, language, "confirm") && ok;
  touch(440 + 100, 300 + 40);   // "No"
  advance(300);

  scene_armed();
  advance(1200);
  ok = save(directory, language, "armed") && ok;

  scene_alarm();
  advance(1200);
  ok = save(directory, language, "alarm") && ok;

  sim::g_scene.control.state = armor::control::State::kPaused;
  advance(1200);
  ok = save(directory, language, "paused") && ok;
  sim::g_scene.control.state = armor::control::State::kRunning;
  advance(600);
  return ok;
}
}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    std::fprintf(stderr, "usage: armor_hmi_screen_sim OUTPUT_DIR [language ...]\n");
    return 2;
  }
  const std::string directory = argv[1];
  std::vector<std::string> languages;
  for (int i = 2; i < argc; ++i) languages.emplace_back(argv[i]);
  if (languages.empty()) languages = {"en", "es", "de", "fr", "it", "ja", "zh", "pt"};

  lv_init();
  lv_tick_set_cb(tick_cb);
  lv_display_t* display = lv_display_create(kWidth, kHeight);
  lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
  lv_display_set_buffers(display, g_buffer, nullptr, sizeof g_buffer, LV_DISPLAY_RENDER_MODE_FULL);
  lv_display_set_flush_cb(display, flush_cb);
  lv_indev_t* pointer = lv_indev_create();
  lv_indev_set_type(pointer, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(pointer, pointer_cb);

  if (!armor::ui::start([] { return g_context; })) {
    std::fprintf(stderr, "the screen could not be built\n");
    return 1;
  }
  bool ok = true;
  for (const std::string& language : languages) ok = take_language(directory, language) && ok;
  std::printf("SCREENS=%s languages=%zu\n", ok ? "PASS" : "FAIL", languages.size());
  return ok ? 0 : 1;
}
