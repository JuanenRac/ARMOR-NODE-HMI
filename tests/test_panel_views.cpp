// ARMOR-NODE-HMI - host tests of what the panel does with the check before arming, the countdown of the delays and the camera picture on demand: the server's answers, the seconds left, the
// beeps, the JPEG it accepts, and the settings that turn them on (all off by default). No server, no camera and no screen were used.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <string>

#include "../core/countdown.hpp"
#include "../core/hmi_config.hpp"
#include "../core/preflight_view.hpp"
#include "../core/server_view.hpp"
#include "../core/still_view.hpp"
#include "../core/view_model.hpp"
#include "check.hpp"

using namespace armor;
using namespace armor::hmi;

static const char* kBase = "\"mode\":\"armed\",\"revision\":7,\"time_ms\":1,\"nodes\":{\"online\":1,\"total\":1},\"alarms\":{\"active\":0,\"unacknowledged\":0,\"items\":[]}";

static void test_summary_delay() {
  Summary s;
  CHECK(parse_summary(std::string("{") + kBase + "}", s) && !s.delay.present);   // an older server sends none
  CHECK(parse_summary(std::string("{") + kBase + ",\"delay\":{\"phase\":\"exit\",\"remaining_s\":25}}", s) && s.delay.present && !s.delay.entry && s.delay.remaining_s == 25);
  CHECK(parse_summary(std::string("{") + kBase + ",\"delay\":{\"phase\":\"entry\",\"remaining_s\":9}}", s) && s.delay.present && s.delay.entry && s.delay.remaining_s == 9);
  // what is not a countdown is not one: an unknown phase, no seconds, none left, a delay that is not an object
  for (const char* bad : {"{\"phase\":\"later\",\"remaining_s\":5}", "{\"phase\":\"exit\"}", "{\"phase\":\"exit\",\"remaining_s\":0}", "{\"phase\":\"exit\",\"remaining_s\":-4}", "5", "[]"}) {
    CHECK(parse_summary(std::string("{") + kBase + ",\"delay\":" + bad + "}", s) && !s.delay.present);
  }
  // a number above an hour is cut by the reader (integer_or refuses it, so it is no countdown)
  CHECK(parse_summary(std::string("{") + kBase + ",\"delay\":{\"phase\":\"exit\",\"remaining_s\":99999}}", s) && (!s.delay.present || s.delay.remaining_s <= kDelayMaxSeconds));
  // the block the screen draws carries it, and a new number is news
  Summary with;
  CHECK(parse_summary(std::string("{") + kBase + ",\"delay\":{\"phase\":\"exit\",\"remaining_s\":25}}", with));
  const ViewModel a = make_view(with, Link::kOnline, true, true, false, "");
  CHECK(a.delay.present && !a.delay.entry && a.delay.remaining_s == 25);
  Summary later = with;
  later.delay.remaining_s = 20;
  const ViewModel b = make_view(later, Link::kOnline, true, true, false, "");
  CHECK(a != b);
  Summary none = with;
  none.delay = Delay{};
  CHECK(!make_view(none, Link::kOnline, true, true, false, "").delay.present);
}

static void test_countdown() {
  Countdown c;
  CHECK(!c.active(0) && c.remaining_s(0) == 0 && c.kind(0) == DelayKind::kNone);
  c.update(true, false, 25, 1000);
  CHECK(c.active(1000) && c.kind(1000) == DelayKind::kExit && c.remaining_s(1000) == 25);
  CHECK(c.remaining_s(1001) == 25 && c.remaining_s(1999) == 25);
  CHECK(c.remaining_s(2000) == 24 && c.remaining_s(13500) == 13 && c.remaining_s(25999) == 1);   // rounded up: 1 until the last moment
  CHECK(c.remaining_s(26000) == 0 && !c.active(26000) && c.kind(26000) == DelayKind::kNone);
  // the clock going backwards counts as no time having passed
  CHECK(c.remaining_s(500) == 25);
  // a new summary replaces what is kept, and an entry delay is told apart
  c.update(true, true, 40, 50000);
  CHECK(c.kind(50000) == DelayKind::kEntry && c.remaining_s(50000) == 40);
  // a summary with no countdown ends it at once; so does one with no seconds
  c.update(false, false, 0, 51000);
  CHECK(!c.active(51000));
  c.update(true, false, 0, 52000);
  CHECK(!c.active(52000));
  // a number above an hour is cut
  c.update(true, false, 100000, 0);
  CHECK(c.remaining_s(0) == kDelayMaxSeconds);
}

static void test_beeps() {
  Countdown c;
  CHECK(!c.beep_due(0, true));   // nothing runs
  c.update(true, false, 20, 0);
  CHECK(!c.beep_due(10, false));   // the setting is off: never
  CHECK(c.beep_due(10, true));      // a new countdown beeps at once
  CHECK(!c.beep_due(500, true) && !c.beep_due(1009, true));
  CHECK(c.beep_due(1010, true));    // one a second
  CHECK(c.beep_due(2010, true) && !c.beep_due(2400, true));
  // the last five seconds: twice a second
  int beeps = 0;
  for (std::int64_t t = 15000; t < 20000; t += 50) if (c.beep_due(t, true)) ++beeps;
  CHECK(beeps >= 9 && beeps <= 11);
  // it is over: no more
  CHECK(!c.beep_due(20000, true) && !c.beep_due(30000, true));
}

static const char* kFindings =
    "{\"ready\":false,\"checked_at\":\"x\",\"blocking\":2,\"warnings\":1,\"issues\":["
    "{\"kind\":\"camera\",\"level\":\"warn\",\"code\":\"camera_down\",\"id\":\"gate\"},"
    "{\"kind\":\"node\",\"level\":\"block\",\"code\":\"node_offline\",\"id\":\"radar-1\"},"
    "{\"kind\":\"device\",\"level\":\"block\",\"code\":\"opening_open\",\"id\":\"front-door\"}]}";

static void test_preflight() {
  Preflight p;
  CHECK(parse_preflight("{\"ready\":true,\"checked_at\":\"x\",\"blocking\":0,\"warnings\":0,\"issues\":[]}", p) && p.valid && p.ready && p.row_count == 0 && p.more == 0);
  CHECK(preflight_shown(true, false, p) == PreflightShown::kAllClear);
  CHECK(parse_preflight(kFindings, p) && p.valid && !p.ready && p.blocking == 2 && p.warnings == 1 && p.row_count == 3);
  // the findings that block come first
  CHECK(std::string(p.rows[0].code) == "node_offline" && p.rows[0].block && std::string(p.rows[0].id) == "radar-1");
  CHECK(std::string(p.rows[1].code) == "opening_open" && p.rows[1].block);
  CHECK(std::string(p.rows[2].code) == "camera_down" && !p.rows[2].block);
  CHECK(preflight_shown(true, false, p) == PreflightShown::kFindings);
  // what the screen shows depends on the setting, on whether the answer is still on its way and on whether there is one
  CHECK(preflight_shown(false, false, p) == PreflightShown::kNothing && preflight_shown(false, true, p) == PreflightShown::kNothing);
  CHECK(preflight_shown(true, true, p) == PreflightShown::kChecking);
  CHECK(preflight_shown(true, false, Preflight{}) == PreflightShown::kNothing);
  // more findings than rows: the others are counted
  std::string many = "{\"ready\":false,\"checked_at\":\"x\",\"blocking\":8,\"warnings\":0,\"issues\":[";
  for (int i = 0; i < 8; ++i) many += std::string(i ? "," : "") + "{\"kind\":\"node\",\"level\":\"block\",\"code\":\"node_offline\",\"id\":\"n" + std::to_string(i) + "\"}";
  many += "]}";
  CHECK(parse_preflight(many, p) && p.row_count == kPreflightRows && p.more == 3 && p.blocking == 8);
  // a long id is cut, and an unknown code has a general text
  CHECK(parse_preflight("{\"ready\":false,\"blocking\":1,\"warnings\":0,\"issues\":[{\"level\":\"block\",\"code\":\"something_new\",\"id\":\"0123456789012345678901234567890123456789012345678901234567890123456789\"}]}", p));
  CHECK(std::string(preflight_key(p.rows[0].code)) == "pf_other" && std::string(p.rows[0].id).size() == kPreflightIdBytes - 1);
  for (const char* code : {"node_offline", "panel_silent", "zone_open", "camera_down", "opening_open", "device_offline", "alarm_unseen"}) CHECK(std::string(preflight_key(code)) != "pf_other");
  // a server that says ready with a blocking finding in the list is not believed
  CHECK(parse_preflight("{\"ready\":true,\"issues\":[{\"level\":\"block\",\"code\":\"zone_open\",\"id\":\"z\"}]}", p) && !p.ready);
  // what is not the server's answer is refused
  for (const char* bad : {"", "[]", "{}", "{\"ready\":\"yes\",\"issues\":[]}", "{\"ready\":true}", "{\"ready\":true,\"issues\":{}}", "not json"}) CHECK(!parse_preflight(bad, p));
  // items without a code are skipped
  CHECK(parse_preflight("{\"ready\":true,\"issues\":[{\"level\":\"warn\"},5,{\"level\":\"warn\",\"code\":\"camera_down\",\"id\":\"c\"}]}", p) && p.row_count == 1 && p.warnings == 1);
}

static std::string jpeg(std::uint8_t sof, int width, int height, bool end = true) {
  std::string out;
  out += "\xFF\xD8";
  out += std::string("\xFF\xE0\x00\x10JFIF\x00\x01\x01\x00\x00\x01\x00\x01\x00\x00", 18);
  out += "\xFF";
  out += static_cast<char>(sof);
  out += std::string("\x00\x0B\x08", 3);
  out += static_cast<char>(height >> 8); out += static_cast<char>(height & 255);
  out += static_cast<char>(width >> 8); out += static_cast<char>(width & 255);
  out += std::string("\x01\x01\x11\x00", 4);
  out += "\xFF\xDA\x00\x02";
  if (end) out += "\xFF\xD9";
  return out;
}

static void test_still_check() {
  std::uint32_t w = 0, h = 0;
  CHECK(check_still(jpeg(0xC0, 640, 360), w, h) == StillCheck::kOk && w == 640 && h == 360);
  CHECK(check_still(jpeg(0xC1, 320, 240), w, h) == StillCheck::kOk && w == 320 && h == 240);
  CHECK(check_still(jpeg(0xC2, 320, 240), w, h) == StillCheck::kProgressive);
  CHECK(check_still(jpeg(0xC0, 1920, 1080), w, h) == StillCheck::kTooManyPixels && w == 1920 && h == 1080);
  CHECK(check_still(jpeg(0xC0, 640, 480), w, h) == StillCheck::kOk);          // exactly the limit
  CHECK(check_still(jpeg(0xC0, 641, 480), w, h) == StillCheck::kTooManyPixels);
  CHECK(check_still(jpeg(0xC0, 0, 480), w, h) == StillCheck::kNoSize);
  CHECK(check_still(jpeg(0xC0, 640, 360, false), w, h) == StillCheck::kTruncated);
  CHECK(check_still("", w, h) == StillCheck::kNotJpeg && check_still("GIF89a....", w, h) == StillCheck::kNotJpeg && check_still("\xFF\xD8", w, h) == StillCheck::kNotJpeg);
  CHECK(check_still(std::string(kStillMaxBytes + 1, 'x'), w, h) == StillCheck::kTooBig);
  // a picture with no frame header at all has no size
  CHECK(check_still(std::string("\xFF\xD8\xFF\xDA\x00\x02\xFF\xD9", 8), w, h) == StillCheck::kNoSize);
  // a segment that claims to be longer than the picture
  CHECK(check_still(std::string("\xFF\xD8\xFF\xE0\x7F\xFF\xFF\xD9", 8), w, h) == StillCheck::kTruncated);
  for (StillCheck c : {StillCheck::kOk, StillCheck::kNotJpeg, StillCheck::kTruncated, StillCheck::kTooBig, StillCheck::kProgressive, StillCheck::kTooManyPixels, StillCheck::kNoSize}) CHECK(std::string(still_check_name(c)).size() > 1);
  CHECK(clamp_refresh_s(0) == kStillMinRefreshS && clamp_refresh_s(1000) == kStillMaxRefreshS && clamp_refresh_s(7) == 7);
}

static void test_still_schedule() {
  StillSchedule s;
  s.configure(5);
  s.opened(1000);
  CHECK(s.due(1000));   // the first one is asked for now
  s.succeeded(1000);
  CHECK(!s.due(5999) && s.due(6000));
  // a failure waits longer each time, up to a minute
  s.failed(10000);
  CHECK(!s.due(14999) && s.due(15000) && s.failures() == 1);
  s.failed(15000);
  CHECK(!s.due(24999) && s.due(25000) && s.failures() == 2);
  for (int i = 0; i < 10; ++i) s.failed(100000);
  CHECK(!s.due(159999) && s.due(160000));   // 60 s at most
  // a 429 waits at least what the server said
  s.opened(0);
  s.failed(0, 12);
  CHECK(!s.due(11999) && s.due(12000));
  // a good picture goes back to the normal pace, and the pace is clamped
  s.succeeded(20000);
  CHECK(s.failures() == 0 && s.due(25000) && !s.due(24999));
  s.configure(1);
  CHECK(s.refresh_s() == kStillMinRefreshS);
}

static void test_settings() {
  config::Settings base = config::default_settings("a1b2c3");
  base.sta.enabled = true;
  base.sta.ssid = "casa";
  base.sta.password = "una-clave-larga";
  // everything is off by default
  CHECK(!base.controls.preflight && !base.controls.countdown_beeps && !base.camera.enabled && base.camera.id.empty() && base.camera.refresh_s == 5);
  // the values load (before the refusals: a refusal test is worthless if the same document with good values does not load)
  config::Settings out;
  config::Problems problems;
  const char* document = "{\"controls\":{\"ack\":\"confirm\",\"preflight\":true,\"countdown_beeps\":true},\"camera\":{\"enabled\":true,\"id\":\"gate-cam.1\",\"refresh_s\":10}}";
  CHECK(config::load(document, base, out, problems) && problems.empty());
  CHECK(out.controls.preflight && out.controls.countdown_beeps && out.controls.ack == AckMode::kConfirm && out.camera.enabled && out.camera.id == "gate-cam.1" && out.camera.refresh_s == 10);
  // the stored form reads back the same
  const std::string stored = config::to_json(out, true);
  CHECK(stored.find("\"preflight\":true") != std::string::npos && stored.find("\"countdown_beeps\":true") != std::string::npos);
  CHECK(stored.find("\"camera\":{\"enabled\":true,\"id\":\"gate-cam.1\",\"refresh_s\":10}") != std::string::npos);
  config::Settings back;
  problems.clear();
  CHECK(config::load(stored, base, back, problems) && back.controls.preflight && back.camera.id == "gate-cam.1" && config::to_json(back, true) == stored);
  CHECK(config::to_json(base, true).find("\"camera\":{\"enabled\":false,\"id\":\"\",\"refresh_s\":5}") != std::string::npos);
  // mistakes are refused with the path
  const auto refuses = [&](const char* text, const char* path) {
    config::Problems p;
    config::Settings o;
    if (config::load(text, base, o, p)) return false;
    for (const config::Problem& item : p) if (item.path == path) return true;
    return false;
  };
  CHECK(refuses("{\"controls\":{\"preflight\":\"yes\"}}", "controls.preflight"));
  CHECK(refuses("{\"controls\":{\"countdown_beeps\":1}}", "controls.countdown_beeps"));
  CHECK(refuses("{\"camera\":{\"enabled\":\"on\"}}", "camera.enabled"));
  CHECK(refuses("{\"camera\":{\"refresh_s\":1}}", "camera.refresh_s"));
  CHECK(refuses("{\"camera\":{\"refresh_s\":61}}", "camera.refresh_s"));
  CHECK(refuses("{\"camera\":{\"id\":\"../admin\"}}", "camera.id"));
  CHECK(refuses("{\"camera\":{\"id\":\"a b\"}}", "camera.id"));
  CHECK(refuses("{\"camera\":{\"id\":5}}", "camera.id"));
}

int main() {
  test_summary_delay();
  test_countdown();
  test_beeps();
  test_preflight();
  test_still_check();
  test_still_schedule();
  test_settings();
  FINISH("panel_views");
}
