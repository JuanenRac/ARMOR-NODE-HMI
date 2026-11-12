// ARMOR-NODE-HMI - host tests of the voice conversation: the order of the steps, the time limits and what a touch does.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <cstdint>
#include <string>

#include "../core/voice_answer.hpp"
#include "../core/voice_challenge.hpp"
#include "../core/voice_session.hpp"
#include "check.hpp"

using namespace armor::voice;

static void test_happy_path() {
  Session s;
  CHECK(s.state() == State::kOff);
  CHECK(s.on_event(Event::kWake, 1000) == Action::kNone);   // off: it never listens
  s.set_enabled(true, 0);
  CHECK(s.state() == State::kIdle);
  CHECK(s.on_event(Event::kWake, 5000) == Action::kStartRecording && s.state() == State::kListening);
  CHECK(s.on_event(Event::kSpeechEnded, 7000) == Action::kStopRecordingAndSend && s.state() == State::kThinking);
  CHECK(s.on_event(Event::kReply, 8000) == Action::kStartSpeaking && s.state() == State::kSpeaking);
  CHECK(s.on_event(Event::kSpeechDone, 11000) == Action::kBackToIdle && s.state() == State::kIdle);
}

static void test_order_is_enforced() {
  Session s;
  s.set_enabled(true, 0);
  CHECK(s.on_event(Event::kSpeechEnded, 5000) == Action::kNone && s.state() == State::kIdle);
  CHECK(s.on_event(Event::kReply, 5000) == Action::kNone);
  CHECK(s.on_event(Event::kSpeechDone, 5000) == Action::kNone);
  CHECK(s.on_event(Event::kWake, 5000) == Action::kStartRecording);
  CHECK(s.on_event(Event::kWake, 5100) == Action::kNone);   // already listening: a second wake word does nothing
  CHECK(s.on_event(Event::kReply, 5200) == Action::kNone);  // a reply nobody asked for
}

static void test_time_limits() {
  Limits l;
  l.listen_ms = 8000; l.think_ms = 15000; l.speak_ms = 30000; l.failure_show_ms = 4000;
  Session s(l);
  s.set_enabled(true, 0);
  s.on_event(Event::kWake, 10000);
  CHECK(s.tick(17999) == Action::kNone && s.state() == State::kListening);
  CHECK(s.tick(18000) == Action::kStopRecordingAndSend && s.state() == State::kThinking);   // the microphone closes by itself
  CHECK(s.tick(32999) == Action::kNone);
  CHECK(s.tick(33000) == Action::kShowFailure && s.state() == State::kFailed && s.failure() == "timeout");
  CHECK(s.tick(36999) == Action::kNone);
  CHECK(s.tick(37000) == Action::kBackToIdle && s.state() == State::kIdle);
  // a reply that never ends must not lock the screen
  s.on_event(Event::kWake, 40000); s.on_event(Event::kSpeechEnded, 41000); s.on_event(Event::kReply, 42000);
  CHECK(s.tick(71999) == Action::kNone && s.tick(72000) == Action::kStopAudio && s.state() == State::kIdle);
}

static void test_touch_and_failure() {
  Session s;
  s.set_enabled(true, 0);
  s.on_event(Event::kWake, 5000);
  CHECK(s.on_event(Event::kTouch, 5500) == Action::kStopAudio && s.state() == State::kIdle);   // a person can always stop it
  CHECK(s.on_event(Event::kTouch, 5600) == Action::kNone);
  s.on_event(Event::kWake, 9000); s.on_event(Event::kSpeechEnded, 9500);
  CHECK(s.on_event(Event::kFailure, 10000, "no_speech") == Action::kShowFailure && s.failure() == "no_speech");
  CHECK(s.on_event(Event::kFailure, 10100) == Action::kNone);   // already failed
  CHECK(s.on_event(Event::kTouch, 10200) == Action::kBackToIdle && s.state() == State::kIdle);   // a touch dismisses the failure
  // idle has nothing to fail
  CHECK(s.on_event(Event::kFailure, 20000) == Action::kNone && s.state() == State::kIdle);
}

static void test_wake_cooldown_and_disable() {
  Session s;
  s.set_enabled(true, 0);
  s.on_event(Event::kWake, 5000); s.on_event(Event::kSpeechEnded, 6000); s.on_event(Event::kReply, 7000);
  CHECK(s.on_event(Event::kSpeechDone, 9000) == Action::kBackToIdle);
  CHECK(s.on_event(Event::kWake, 9500) == Action::kNone);               // the speaker's own sound is not a new request
  CHECK(s.on_event(Event::kWake, 10600) == Action::kStartRecording);    // after the cool-down it is
  s.set_enabled(false, 11000);
  CHECK(s.state() == State::kOff && s.on_event(Event::kWake, 20000) == Action::kNone);
  CHECK(std::string(to_text(State::kThinking)) == "thinking" && std::string(to_text(State::kOff)) == "off");
}

// The answer of the service carries the audio in base64: a spoken reply is far longer than the 64 KB the JSON reader accepts, and is read all the same.
static void test_answer_with_long_audio() {
  Answer a;
  CHECK(parse_answer(R"({"heard":"what time is it","reply":"It is noon.","audio":"QUJD"})", a) && a.heard == "what time is it" && a.reply == "It is noon." && a.audio_base64 == "QUJD");
  std::string audio(250 * 1024, 'A');
  CHECK(parse_answer("{\"heard\":\"hola\",\"reply\":\"Son las doce.\",\"audio\":\"" + audio + "\"}", a) && a.reply == "Son las doce." && a.audio_base64.size() == audio.size());
  CHECK(parse_answer("{\"audio\":\"" + audio + "\",\"reply\":\"ok\"}", a) && a.reply == "ok" && a.audio_base64.size() == audio.size());   // the audio first
  CHECK(parse_answer(R"({"heard":"x","reply":"y","audio":null})", a) && a.audio_base64.empty());
  CHECK(parse_answer(R"({"heard":"x","reply":"y"})", a) && a.audio_base64.empty());
  CHECK(parse_answer(R"({"reply":"a\/b","audio":"AB\/C"})", a) && a.audio_base64 == "AB/C");   // the escape of a slash in the audio is undone
  // the key inside a sentence is a sentence
  CHECK(parse_answer(R"({"heard":"say \"audio\": please","reply":"fine","audio":"QQ=="})", a) && a.audio_base64 == "QQ==" && a.heard == "say \"audio\": please");
  // what is not an answer is refused
  CHECK(!parse_answer("", a) && !parse_answer("[]", a) && !parse_answer(R"({"audio":5})", a) && !parse_answer(R"({"audio":"unfinished)", a) && !parse_answer(R"({"reply":"x",})", a));
}

// The number of the spoken challenge: shown on the screen, never said aloud, and a number that is not well formed makes the answer bad so that silencing stays closed.
static void test_challenge() {
  CHECK(valid_challenge("10") && valid_challenge("47") && valid_challenge("99") && valid_challenge("123456"));
  CHECK(!valid_challenge("") && !valid_challenge("4") && !valid_challenge("1234567") && !valid_challenge("4a") && !valid_challenge(" 47") && !valid_challenge("-47") && !valid_challenge("4.7"));
  Answer a;
  CHECK(parse_answer(R"({"heard":"silence","reply":"Say the number shown on the screen.","challenge":"47"})", a) && a.challenge == "47" && a.reply.find("47") == std::string::npos);
  CHECK(parse_answer(R"({"heard":"silence","reply":"ok","audio":"QUJD","challenge":"83"})", a) && a.challenge == "83" && a.audio_base64 == "QUJD");
  CHECK(parse_answer(R"({"heard":"x","reply":"y"})", a) && a.challenge.empty());
  CHECK(!parse_answer(R"({"heard":"x","reply":"y","challenge":47})", a) && !parse_answer(R"({"heard":"x","reply":"y","challenge":"4"})", a) && !parse_answer(R"({"heard":"x","reply":"y","challenge":null})", a));
  // shown for a little less than the service keeps it, then taken away; a reply without a number takes it away at once
  ChallengeShown shown;
  CHECK(shown.current(0).empty());
  shown.set("47", 1000);
  CHECK(shown.current(1000) == "47" && shown.current(1000 + ChallengeShown::kShowMs - 1) == "47" && shown.current(1000 + ChallengeShown::kShowMs).empty());
  shown.set("47", 5000);
  shown.set("", 6000);
  CHECK(shown.current(6001).empty());
  shown.set("12", 7000);
  shown.set("bad", 7100);
  CHECK(shown.current(7200).empty());
  shown.set("12", 8000);
  shown.clear();
  CHECK(shown.current(8001).empty());
  // the clock of the chip wraps around: only the difference counts
  const std::uint32_t before_wrap = 0xFFFFFFF0u;
  std::uint32_t after_wrap = before_wrap, long_after = before_wrap;
  after_wrap += 100u;
  long_after += ChallengeShown::kShowMs + 1u;
  shown.set("55", before_wrap);
  CHECK(shown.current(after_wrap) == "55" && shown.current(long_after).empty());
}

static void test_panel_header() {
  CHECK(panel_header_value("hmi-a1b2c3") == "hmi-a1b2c3");
  CHECK(panel_header_value("hall_2") == "hall_2");
  CHECK(panel_header_value("").empty() && panel_header_value("Hall").empty() && panel_header_value("a b").empty() && panel_header_value("a\r\nX-Evil: 1").empty());
  CHECK(panel_header_value(std::string(65, 'a')).empty() && panel_header_value(std::string(64, 'a')).size() == 64);
}

// What blocks the voice task (the microphone, the speaker) asks still() between its chunks: a touch, the time limit and a switch-off end it at once.
static void test_still_while_blocked() {
  Limits l;
  l.listen_ms = 8000; l.think_ms = 15000; l.speak_ms = 30000;
  Session s(l);
  s.set_enabled(true, 0);
  s.on_event(Event::kWake, 10000);
  CHECK(s.still(State::kListening, 10100) && s.still(State::kListening, 17999));
  CHECK(!s.still(State::kListening, 18000) && s.state() == State::kThinking);   // the time of listening ran out while the task was recording
  s.on_event(Event::kReply, 19000);
  CHECK(s.still(State::kSpeaking, 19100) && s.still(State::kSpeaking, 48999));
  CHECK(!s.still(State::kSpeaking, 49000) && s.state() == State::kIdle);        // a reply that never ends lets go of the speaker
  s.on_event(Event::kWake, 60000);
  s.on_event(Event::kSpeechEnded, 61000);
  s.on_event(Event::kReply, 62000);
  CHECK(s.still(State::kSpeaking, 62500));
  s.on_event(Event::kTouch, 63000);
  CHECK(!s.still(State::kSpeaking, 63100));                                      // a touch stops the sound that is playing
  s.set_enabled(false, 64000);
  CHECK(!s.still(State::kListening, 64100) && !s.still(State::kSpeaking, 64100));
}

// The power goes at random moments of a long random conversation: a new session knows nothing (it is off until the panel switches the voice on), so it never records, plays or shows a failure
// by itself, and from then on every action follows the order of the steps whatever the events do.
static void test_random_power_cuts() {
  std::uint32_t seed = 2026;
  const auto next = [&seed]() { seed = seed * 1664525u + 1013904223u; return seed >> 8; };
  int cuts = 0, conversations = 0;
  for (int round = 0; round < 60; ++round) {
    Session session;
    session.set_enabled(true, 0);
    std::uint32_t now = 0;
    for (int step = 0; step < 2000; ++step) {
      now += next() % 900;
      if (next() % 25 == 0) {   // the power goes
        ++cuts;
        session = Session();
        CHECK(session.state() == State::kOff);
        CHECK(session.on_event(Event::kWake, now) == Action::kNone && session.on_event(Event::kReply, now) == Action::kNone && session.on_event(Event::kFailure, now, "late") == Action::kNone);
        CHECK(session.tick(now + 100000) == Action::kNone && session.state() == State::kOff);   // no timer runs in a session that is off
        now += 1 + next() % 5000;
        session.set_enabled(true, now);
        CHECK(session.state() == State::kIdle);
        CHECK(session.on_event(Event::kSpeechEnded, now) == Action::kNone && session.on_event(Event::kSpeechDone, now) == Action::kNone && session.on_event(Event::kTouch, now) == Action::kNone);   // nothing to end
        continue;
      }
      const State before = session.state();
      Action action = Action::kNone;
      switch (next() % 7) {
        case 0: action = session.on_event(Event::kWake, now); break;
        case 1: action = session.on_event(Event::kSpeechEnded, now); break;
        case 2: action = session.on_event(Event::kReply, now); break;
        case 3: action = session.on_event(Event::kSpeechDone, now); break;
        case 4: action = session.on_event(Event::kFailure, now, "x"); break;
        case 5: action = session.on_event(Event::kTouch, now); break;
        default: action = session.tick(now); break;
      }
      const State after = session.state();
      if (action == Action::kStartRecording) { CHECK(before == State::kIdle && after == State::kListening); ++conversations; }
      if (action == Action::kStopRecordingAndSend) CHECK(before == State::kListening && after == State::kThinking);
      if (action == Action::kStartSpeaking) CHECK(before == State::kThinking && after == State::kSpeaking);
      if (action == Action::kShowFailure) CHECK(before != State::kIdle && before != State::kOff && after == State::kFailed);
      if (action == Action::kNone && before != State::kFailed) CHECK(after == before || (before == State::kListening && after == State::kThinking));
      CHECK(after != State::kOff);   // nothing in the conversation switches the voice off by itself
    }
  }
  CHECK(cuts > 1000 && conversations > 500);
}

int main() {
  test_still_while_blocked();
  test_random_power_cuts();
  test_panel_header();
  test_answer_with_long_audio();
  test_challenge();
  test_happy_path();
  test_order_is_enforced();
  test_time_limits();
  test_touch_and_failure();
  test_wake_cooldown_and_disable();
  FINISH("test_voice");
}
