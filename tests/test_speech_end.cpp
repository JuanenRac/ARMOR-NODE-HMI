// ARMOR-NODE-HMI - host tests of where a spoken request ends (core/speech_end.hpp): the room is measured, a question is told from a click, the recording stops after the quiet that follows
// the last word and never in the pauses between words, nobody speaking is "no speech", and the settings. The microphone is simulated: noise of a room, bursts of voice, a click.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

#include "../core/hmi_config.hpp"
#include "../core/speech_end.hpp"

static int failures = 0;
static int checks = 0;
#define CHECK(condition)                                                              \
  do {                                                                                \
    ++checks;                                                                         \
    if (!(condition)) {                                                               \
      ++failures;                                                                     \
      std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #condition);                \
    }                                                                                 \
  } while (0)

using namespace armor::hmi;

// ---- the simulated microphone --------------------------------------------------------------------------------------------------------

// A recording of segments: {milliseconds, level of the sound as RMS}. The room is random noise of its level; a voice is a mix of two tones with the noise of the room under it.
struct Segment { int ms; double level; bool voice; };

static std::vector<std::int16_t> make(const std::vector<Segment>& segments, double room, unsigned rate = 16000, std::uint32_t seed = 11) {
  std::vector<std::int16_t> out;
  std::uint32_t state = seed;
  const auto noise = [&](double rms) { state = state * 1664525u + 1013904223u; return (static_cast<double>(state >> 8) / 16777216.0 - 0.5) * 2.0 * rms * 1.7320508; };   // uniform with that RMS
  double phase = 0;
  for (const Segment& s : segments) {
    const std::size_t n = static_cast<std::size_t>(s.ms) * rate / 1000;
    for (std::size_t i = 0; i < n; ++i) {
      double v = noise(s.voice ? room : s.level);
      if (s.voice) { phase += 2.0 * M_PI * 180.0 / rate; v += s.level * 1.4142 * (0.7 * std::sin(phase) + 0.3 * std::sin(3.1 * phase)) / std::sqrt(0.5 * (0.49 + 0.09)) * 0.7071; }
      out.push_back(static_cast<std::int16_t>(std::max(-32000.0, std::min(32000.0, v))));
    }
  }
  return out;
}

struct Outcome { SpeechVerdict verdict; double at_ms; bool started; };

// Feeds a recording in frames of 20 ms, as the microphone gives it, until the detector says something.
static Outcome run(const std::vector<std::int16_t>& audio, const SpeechEndSettings& settings, unsigned rate = 16000) {
  SpeechEnd detector(settings, rate);
  const std::size_t frame = rate / 50;
  for (std::size_t at = 0; at + frame <= audio.size(); at += frame) {
    if (detector.feed(audio.data() + at, frame) != SpeechVerdict::kContinue) return {detector.verdict(), detector.elapsed_ms(), detector.speech_started()};
  }
  return {detector.verdict(), detector.elapsed_ms(), detector.speech_started()};
}

static SpeechEndSettings on() { SpeechEndSettings s; s.enabled = true; return s; }

static void test_the_settings() {
  SpeechEndSettings s;
  CHECK(!s.enabled && std::string(check(s)).empty());   // off, and valid, by default
  const auto bad = [](SpeechEndSettings x, const char* member) { return std::string(check(x)) == member; };
  { auto x = s; x.silence_ms = 299; CHECK(bad(x, "silence_ms")); }
  { auto x = s; x.silence_ms = 3001; CHECK(bad(x, "silence_ms")); }
  { auto x = s; x.min_speech_ms = 79; CHECK(bad(x, "min_speech_ms")); }
  { auto x = s; x.level_percent = 149; CHECK(bad(x, "level_percent")); }
  { auto x = s; x.min_level = 49; CHECK(bad(x, "min_level")); }
  { auto x = s; x.min_level = 4001; CHECK(bad(x, "min_level")); }
  { auto x = s; x.lead_ms = 999; CHECK(bad(x, "lead_ms")); }
  { auto x = s; x.calibrate_ms = 99; CHECK(bad(x, "calibrate_ms")); }
  { auto x = s; x.lead_ms = 1000; x.calibrate_ms = 1000; CHECK(bad(x, "calibrate_ms")); }   // the room is measured before the wait ends
}

static void test_a_question_ends_after_the_quiet() {
  const std::vector<Segment> question = {{400, 60, false}, {700, 3000, true}, {150, 60, false}, {600, 2800, true}, {2500, 60, false}};
  const std::vector<std::int16_t> audio = make(question, 60);
  const Outcome r = run(audio, on());
  CHECK(r.verdict == SpeechVerdict::kSpeechEnded && r.started);
  // the voice stops at 1850 ms; 900 ms of quiet later (give or take a frame and the soft end of a word) the recording stops
  CHECK(r.at_ms >= 1850 + 800 && r.at_ms <= 1850 + 1050);
  // it stops long before the end of the recording (4.4 s here, and 8 s is the usual limit)
  CHECK(r.at_ms < 3200);
  // the quiet time is a setting
  SpeechEndSettings quick = on();
  quick.silence_ms = 400;
  const Outcome q = run(audio, quick);
  CHECK(q.verdict == SpeechVerdict::kSpeechEnded && q.at_ms >= 1850 + 350 && q.at_ms <= 1850 + 600);
}

static void test_the_pauses_between_words_do_not_end_it() {
  // 700 ms pauses between the words, under the 900 ms of quiet: one request
  const std::vector<Segment> slow = {{400, 60, false}, {500, 3000, true}, {700, 60, false}, {500, 3000, true}, {700, 60, false}, {500, 3000, true}, {2000, 60, false}};
  const Outcome r = run(make(slow, 60), on());
  CHECK(r.verdict == SpeechVerdict::kSpeechEnded && r.at_ms >= 2800 + 800);   // after the last word at 2.8 s, not in the middle
  // a longer pause than the quiet ends the request at it: that is what the setting means
  const std::vector<Segment> cut = {{400, 60, false}, {500, 3000, true}, {1200, 60, false}, {500, 3000, true}, {2000, 60, false}};
  const Outcome c = run(make(cut, 60), on());
  CHECK(c.verdict == SpeechVerdict::kSpeechEnded && c.at_ms < 1400 + 1100);
}

static void test_nobody_speaks() {
  const Outcome r = run(make({{8000, 60, false}}, 60), on());
  CHECK(r.verdict == SpeechVerdict::kNoSpeech && !r.started && r.at_ms >= 4000 && r.at_ms <= 4040);   // lead_ms, the wait for a first word
  // a click is not a question
  const Outcome click = run(make({{600, 60, false}, {40, 5000, true}, {7000, 60, false}}, 60), on());
  CHECK(click.verdict == SpeechVerdict::kNoSpeech && !click.started);
  // nor are noises now and then: a door every second does not add up to a request
  std::vector<Segment> doors;
  doors.push_back({500, 60, false});
  for (int i = 0; i < 6; ++i) { doors.push_back({50, 4000, true}); doors.push_back({950, 60, false}); }
  const Outcome d = run(make(doors, 60), on());
  CHECK(d.verdict == SpeechVerdict::kNoSpeech && !d.started);
  // the wait for the first word is a setting
  SpeechEndSettings patient = on();
  patient.lead_ms = 7000;
  const Outcome p = run(make({{9000, 60, false}}, 60), patient);
  CHECK(p.verdict == SpeechVerdict::kNoSpeech && p.at_ms >= 7000 && p.at_ms <= 7040);
  // someone who starts late but within the wait is heard
  const Outcome late = run(make({{3000, 60, false}, {700, 3000, true}, {3000, 60, false}}, 60), on());
  CHECK(late.verdict == SpeechVerdict::kSpeechEnded && late.started);
}

static void test_a_noisy_room() {
  // a room with a fan (RMS 700): the voice has to be 3 times louder, 2100, and a voice of 4000 is
  const Outcome loud = run(make({{500, 700, false}, {900, 4000, true}, {3000, 700, false}}, 700), on());
  CHECK(loud.verdict == SpeechVerdict::kSpeechEnded && loud.started);
  // the same room with nobody talking: no speech, however loud it is
  const Outcome fan = run(make({{9000, 700, false}}, 700), on());
  CHECK(fan.verdict == SpeechVerdict::kNoSpeech);
  // a murmur in that room (1200, under the 2100 a voice needs) is not a request: the setting is how far above the room a voice must be
  const Outcome murmur = run(make({{500, 700, false}, {1500, 1200, true}, {7000, 700, false}}, 700), on());
  CHECK(murmur.verdict == SpeechVerdict::kNoSpeech);
  SpeechEndSettings sensitive = on();
  sensitive.level_percent = 150;
  const Outcome heard = run(make({{500, 700, false}, {1500, 1700, true}, {3000, 700, false}}, 700), sensitive);
  CHECK(heard.verdict == SpeechVerdict::kSpeechEnded && heard.started);
  // a very quiet room: the voice still has to reach the least level, whatever the room
  const Outcome whisper = run(make({{500, 5, false}, {1500, 120, true}, {7000, 5, false}}, 5), on());
  CHECK(whisper.verdict == SpeechVerdict::kNoSpeech);   // 120 is under min_level (200)
  SpeechEndSettings keen = on();
  keen.min_level = 80;
  const Outcome heard_whisper = run(make({{500, 5, false}, {1500, 400, true}, {3000, 5, false}}, 5), keen);
  CHECK(heard_whisper.verdict == SpeechVerdict::kSpeechEnded);
}

static void test_a_person_who_starts_at_once() {
  // the voice begins in the very first frame: the room is measured under it, and the cap keeps the threshold from being the voice itself
  const std::vector<Segment> eager = {{1200, 3000, true}, {3000, 60, false}};
  const Outcome r = run(make(eager, 60), on());
  CHECK(r.verdict == SpeechVerdict::kSpeechEnded && r.started);
  CHECK(r.at_ms >= 1200 + 800 && r.at_ms <= 1200 + 1100);
}

static void test_frames_of_any_size_and_other_rates() {
  const std::vector<Segment> question = {{400, 60, false}, {700, 3000, true}, {2500, 60, false}};
  const std::vector<std::int16_t> audio = make(question, 60);
  const Outcome reference = run(audio, on());
  // frames of 5, 20 and 64 ms give the same verdict within a frame
  for (const std::size_t frame : {80u, 320u, 1024u}) {
    SpeechEnd detector(on(), 16000);
    for (std::size_t at = 0; at + frame <= audio.size() && detector.verdict() == SpeechVerdict::kContinue; at += frame) detector.feed(audio.data() + at, frame);
    CHECK(detector.verdict() == SpeechVerdict::kSpeechEnded && std::fabs(detector.elapsed_ms() - reference.at_ms) <= 2.0 * 1000.0 * static_cast<double>(frame) / 16000.0 + 40.0);
  }
  // 8 kHz audio
  const Outcome narrow = run(make(question, 60, 8000), on(), 8000);
  CHECK(narrow.verdict == SpeechVerdict::kSpeechEnded && std::fabs(narrow.at_ms - reference.at_ms) <= 60.0);
  // once decided, nothing changes the verdict, and nothing in nothing changes anything
  SpeechEnd done(on());
  const std::vector<std::int16_t> silence(320, 0);
  for (int i = 0; i < 400; ++i) done.feed(silence.data(), silence.size());
  CHECK(done.verdict() == SpeechVerdict::kNoSpeech);
  const double at = done.elapsed_ms();
  done.feed(audio.data(), 320);
  CHECK(done.verdict() == SpeechVerdict::kNoSpeech && done.elapsed_ms() == at);
  SpeechEnd empty(on());
  CHECK(empty.feed(nullptr, 0) == SpeechVerdict::kContinue && empty.elapsed_ms() == 0.0 && SpeechEnd::rms(nullptr, 0) == 0.0);
  // the level of a frame
  const std::vector<std::int16_t> square(100, 1000);
  CHECK(std::fabs(SpeechEnd::rms(square.data(), square.size()) - 1000.0) < 1e-9);
}

// The settings document: off by default, written back, refused by name when out of range, left alone when not mentioned.
static void test_the_document() {
  armor::config::Settings s = armor::config::default_settings("a1b2c3");
  s.sta.enabled = true; s.sta.ssid = "casa"; s.sta.password = "una-clave-larga";
  CHECK(!s.voice.end_of_speech.enabled && armor::config::validate(s).empty());
  const std::string text = armor::config::to_json(s, true);
  CHECK(text.find("\"end_of_speech\":false") != std::string::npos && text.find("\"silence_ms\":900") != std::string::npos && text.find("\"calibrate_ms\":300") != std::string::npos);
  armor::config::Settings out;
  armor::config::Problems problems;
  CHECK(armor::config::load("{\"voice\":{\"end_of_speech\":true,\"silence_ms\":1200,\"min_speech_ms\":300,\"level_percent\":250,\"min_level\":150,\"lead_ms\":6000,\"calibrate_ms\":400}}", s, out, problems));
  const SpeechEndSettings& e = out.voice.end_of_speech;
  CHECK(e.enabled && e.silence_ms == 1200 && e.min_speech_ms == 300 && e.level_percent == 250 && e.min_level == 150 && e.lead_ms == 6000 && e.calibrate_ms == 400);
  armor::config::Settings back;
  problems.clear();
  CHECK(armor::config::load(armor::config::to_json(out, true), s, back, problems) && back.voice.end_of_speech.silence_ms == 1200 && back.voice.end_of_speech.enabled);
  problems.clear();
  armor::config::Settings kept;
  CHECK(armor::config::load("{\"voice\":{\"listen_s\":9}}", out, kept, problems) && kept.voice.end_of_speech.enabled && kept.voice.end_of_speech.silence_ms == 1200);   // not mentioned: as it was
  const auto refused = [&](const std::string& body, const std::string& path, const std::string& code) {
    armor::config::Settings x;
    armor::config::Problems p;
    const bool loaded = armor::config::load("{\"voice\":{" + body + "}}", s, x, p);
    for (const armor::config::Problem& problem : p) if (problem.path == path && problem.code == code) return !loaded;
    return false;
  };
  CHECK(refused("\"silence_ms\":100", "voice.silence_ms", "range") && refused("\"min_speech_ms\":9000", "voice.min_speech_ms", "range") && refused("\"level_percent\":50", "voice.level_percent", "range"));
  CHECK(refused("\"min_level\":10", "voice.min_level", "range") && refused("\"lead_ms\":100", "voice.lead_ms", "range") && refused("\"calibrate_ms\":50", "voice.calibrate_ms", "range"));
  CHECK(refused("\"end_of_speech\":\"yes\"", "voice.end_of_speech", "invalid") && refused("\"silence_ms\":\"long\"", "voice.silence_ms", "invalid"));
  CHECK(refused("\"lead_ms\":1000,\"calibrate_ms\":1000", "voice.calibrate_ms", "range"));   // the room is measured before the wait ends
}

int main() {
  test_the_settings();
  test_the_document();
  test_a_question_ends_after_the_quiet();
  test_the_pauses_between_words_do_not_end_it();
  test_nobody_speaks();
  test_a_noisy_room();
  test_a_person_who_starts_at_once();
  test_frames_of_any_size_and_other_rates();
  std::printf("%d checks, %d failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}
