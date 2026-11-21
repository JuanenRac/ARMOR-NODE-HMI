// ARMOR-NODE-HMI - host tests of the edges of the screens that are not generated: the PIN pad, the refusal of a PIN, the alarm audio, the voice conversation and the age of the server's data.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <cstdint>
#include <string>
#include <vector>

#include "../core/alarm_clip.hpp"
#include "../core/pin_entry.hpp"
#include "../core/server_view.hpp"
#include "../core/voice_session.hpp"
#include "check.hpp"

using namespace armor;

// a small deterministic generator (fixed seed), so a failure can be repeated
static std::uint32_t rng_state = 0x1badb002u;
static std::uint32_t next_random() {
  rng_state = rng_state * 1664525u + 1013904223u;
  return rng_state >> 8;
}

static hmi::AlarmLine line(const char* id, hmi::Severity severity, const char* source = "electrical") {
  hmi::AlarmLine alarm;
  alarm.id = id;
  alarm.code = "grid_lost";
  alarm.source_type = source;
  alarm.severity = severity;
  return alarm;
}

static void pin_pad() {
  hmi::PinEntry pin;
  CHECK(pin.empty() && pin.digits().empty() && pin.masked().empty());
  pin.erase_last();                                   // nothing to erase is not an error
  CHECK(pin.empty());
  CHECK(!pin.add('a') && !pin.add(' ') && !pin.add('/') && !pin.add(':') && !pin.add('\0') && !pin.add(static_cast<char>(0xB2)));
  CHECK(pin.empty());
  for (char key = '0'; key <= '9'; ++key) CHECK(pin.add(key));
  CHECK(pin.digits() == "0123456789" && pin.masked() == "**********");
  CHECK(!pin.add('1') && pin.digits().size() == hmi::PinEntry::kMaxDigits);   // the eleventh digit is ignored
  pin.erase_last();
  CHECK(pin.digits() == "012345678" && pin.add('9') && pin.digits() == "0123456789");
  for (char shown : pin.masked()) CHECK(shown == '*');   // the screen never shows a digit
  pin.clear();
  CHECK(pin.empty() && pin.masked().empty());
  CHECK(pin.add('7') && pin.digits() == "7");

  // anything typed in any order keeps the invariants (fixed seed)
  for (int round = 0; round < 200; ++round) {
    hmi::PinEntry fuzz;
    for (int step = 0; step < 40; ++step) {
      const std::uint32_t pick = next_random() % 12;
      if (pick < 8) fuzz.add(static_cast<char>('0' + static_cast<int>(next_random() % 10)));
      else if (pick == 8) fuzz.add(static_cast<char>(next_random() & 0x7f));
      else if (pick == 9) fuzz.erase_last();
      else if (pick == 10 && round % 7 == 0) fuzz.clear();
      CHECK(fuzz.digits().size() <= hmi::PinEntry::kMaxDigits);
      CHECK(fuzz.masked().size() == fuzz.digits().size());
      for (char c : fuzz.digits()) CHECK(c >= '0' && c <= '9');
    }
  }
}

static void confirmation_expiry() {
  // an open confirmation closes after thirty seconds, and at once when the screen sleeps
  CHECK(!hmi::confirm_expired(1000, 1000, false));
  CHECK(!hmi::confirm_expired(1000, 1000 + hmi::kConfirmOpenMs - 1, false));
  CHECK(hmi::confirm_expired(1000, 1000 + hmi::kConfirmOpenMs, false));
  CHECK(hmi::confirm_expired(1000, 1000 + hmi::kConfirmOpenMs + 5000, false));
  CHECK(hmi::confirm_expired(1000, 1001, true));
  CHECK(hmi::confirm_expired(1000, 1000, true));
  CHECK(!hmi::confirm_expired(0, 0, false));
  CHECK(!hmi::confirm_expired(5'000'000'000LL, 5'000'000'000LL + 29999, false));   // a clock past 32 bits is not mistaken
  CHECK(hmi::confirm_expired(5'000'000'000LL, 5'000'000'000LL + 30000, false));
}

static void pin_refusals() {
  using hmi::pin_error_of;
  CHECK(pin_error_of(403, "{\"code\":\"wrong_pin\"}") == "wrong_pin");
  CHECK(pin_error_of(429, "{\"code\":\"pin_locked\"}") == "pin_locked");
  CHECK(pin_error_of(403, "{\"code\":\"pin_not_set\"}") == "pin_not_set");
  CHECK(pin_error_of(403, "{ \"error\" : \"x\" , \"code\" :  \"wrong_pin\" }") == "wrong_pin");   // the spacing does not matter
  CHECK(pin_error_of(403, "{\"code\":\"wrong_pin\",\"error\":\"pin_locked\"}") == "wrong_pin");   // only the code is looked at
  CHECK(pin_error_of(403, "{\"error\":\"wrong_pin\"}").empty());                                  // the words of "error" are not read
  CHECK(pin_error_of(403, "{\"code\":\"Wrong_Pin\"}").empty());
  CHECK(pin_error_of(403, "{\"code\":\"wrong_pin \"}").empty());
  CHECK(pin_error_of(403, "{\"code\":\"other\"}").empty());
  CHECK(pin_error_of(403, "{\"code\":7}").empty());
  CHECK(pin_error_of(403, "{\"code\":null}").empty());
  CHECK(pin_error_of(403, "{\"code\":[\"wrong_pin\"]}").empty());
  CHECK(pin_error_of(403, "[\"wrong_pin\"]").empty());
  CHECK(pin_error_of(403, "\"wrong_pin\"").empty());
  CHECK(pin_error_of(403, "wrong_pin").empty());
  CHECK(pin_error_of(403, "").empty());
  CHECK(pin_error_of(403, "{").empty());
  CHECK(pin_error_of(403, "{\"code\":\"wrong_pin\"").empty());
  for (int status : {0, 200, 204, 301, 400, 401, 404, 422, 500, 502, 503, -1}) CHECK(pin_error_of(status, "{\"code\":\"wrong_pin\"}").empty());   // only 403 and 429 are about a PIN
  CHECK(pin_error_of(429, "{\"code\":\"wrong_pin\"}") == "wrong_pin");

  // truncated answers (a cut link) never crash and never claim a code
  const std::string whole = "{\"error\":\"too many tries\",\"code\":\"pin_locked\"}";
  for (std::size_t cut = 0; cut < whole.size(); ++cut) {
    const std::string got = pin_error_of(429, whole.substr(0, cut));
    CHECK(got.empty() || got == "pin_locked");
  }
  CHECK(pin_error_of(429, whole) == "pin_locked");
  // random bytes: no code comes out of noise (fixed seed)
  for (int round = 0; round < 300; ++round) {
    std::string noise;
    const std::size_t length = next_random() % 64;
    for (std::size_t i = 0; i < length; ++i) noise.push_back(static_cast<char>(next_random() & 0xff));
    CHECK(pin_error_of(403, noise).empty());
  }
}

static void alarm_audio() {
  using clip::Check;
  const std::string good(clip::kMinBytes, '\x05');
  const std::string digest = sha256::hex_of(good);
  CHECK(clip::check(good, digest, clip::kDefaultMaxBytes) == Check::kOk);
  CHECK(clip::check(good, digest, clip::kMinBytes) == Check::kOk);                                  // exactly the limit
  CHECK(clip::check(good, digest, clip::kMinBytes - 1) == Check::kTooLarge);
  CHECK(clip::check(std::string(clip::kMinBytes - 2, '\x05'), digest, clip::kDefaultMaxBytes) == Check::kTooSmall);
  CHECK(clip::check(std::string(), digest, clip::kDefaultMaxBytes) == Check::kEmpty);
  CHECK(clip::check(good + "x", digest, clip::kDefaultMaxBytes) == Check::kOddSize);
  CHECK(clip::check(good, "", clip::kDefaultMaxBytes) == Check::kNoHash);
  CHECK(clip::check(good, "ABC", clip::kDefaultMaxBytes) == Check::kBadHash);
  std::string upper = digest;
  upper[0] = static_cast<char>(upper[0] >= 'a' && upper[0] <= 'f' ? upper[0] - 32 : upper[0]);
  if (upper != digest) CHECK(clip::check(good, upper, clip::kDefaultMaxBytes) == Check::kBadHash);   // the server says it in lower case
  std::string wrong = digest;
  wrong[63] = wrong[63] == '0' ? '1' : '0';
  CHECK(clip::check(good, wrong, clip::kDefaultMaxBytes) == Check::kHashMismatch);
  // the setting never lifts the ceiling
  const std::string big(clip::kMaxBytesCeiling + 2, '\x01');
  CHECK(clip::check(big, sha256::hex_of(big), clip::kMaxBytesCeiling * 4) == Check::kTooLarge);
  const std::string ceiling(clip::kMaxBytesCeiling, '\x01');
  CHECK(clip::check(ceiling, sha256::hex_of(ceiling), clip::kMaxBytesCeiling * 4) == Check::kOk);
  CHECK(std::string(clip::check_name(Check::kHashMismatch)) == "hash_mismatch" && std::string(clip::check_name(Check::kOk)) == "ok");

  // samples are little-endian and signed
  const std::string two("\x01\x80\xff\x7f", 4);
  const std::vector<std::int16_t> samples = clip::samples_of(two);
  CHECK(samples.size() == 2 && samples[0] == -32767 && samples[1] == 32767);
  CHECK(clip::samples_of(std::string()).empty() && clip::samples_of("x").empty());

  // the cache is bounded in entries and in bytes, and the least recently used leaves first
  clip::Cache cache(2, 100);
  cache.put("a", std::vector<std::int16_t>(10));
  cache.put("b", std::vector<std::int16_t>(10));
  CHECK(cache.size() == 2 && cache.bytes() == 40);
  CHECK(cache.find("a") != nullptr);                       // a is now the most recent
  cache.put("c", std::vector<std::int16_t>(10));
  CHECK(cache.find("b") == nullptr && cache.find("a") != nullptr && cache.find("c") != nullptr);
  cache.put("d", std::vector<std::int16_t>(51));           // 102 bytes: more than the whole cache, so not kept
  CHECK(cache.find("d") == nullptr && cache.size() == 2);
  cache.put("a", std::vector<std::int16_t>(20));           // replaced, not duplicated
  CHECK(cache.size() == 2 && cache.bytes() == 60);
  cache.put("e", std::vector<std::int16_t>(50));           // 100 bytes: everything else leaves
  CHECK(cache.size() == 1 && cache.find("e") != nullptr && cache.bytes() == 100);
  clip::Cache none(0, 100);
  none.put("a", std::vector<std::int16_t>(1));
  CHECK(none.size() == 0 && none.find("a") == nullptr);

  // what is said and what is played: the audio never replaces a more serious tone of another origin
  std::array<hmi::ToneChoice, hmi::kSourceKinds> by_source;
  by_source.fill(hmi::ToneChoice::kAuto);
  CHECK(!clip::plan_sound({}, 0, by_source).clip.has_value() && clip::plan_sound({}, 0, by_source).cue == hmi::Cue::kNone);
  by_source[static_cast<std::size_t>(hmi::source_kind("solar"))] = hmi::ToneChoice::kSpoken;
  const clip::Plan quiet = clip::plan_sound({line("1", hmi::Severity::kWarning, "solar")}, 0, by_source);
  CHECK(quiet.clip.has_value() && quiet.clip->kind == clip::Kind::kSpoken && quiet.clip->code == "grid_lost");
  const clip::Plan overruled = clip::plan_sound({line("1", hmi::Severity::kWarning, "solar"), line("2", hmi::Severity::kCritical, "electrical")}, 0, by_source);
  CHECK(!overruled.clip.has_value() && overruled.cue == hmi::Cue::kSiren);
  by_source[static_cast<std::size_t>(hmi::source_kind("solar"))] = hmi::ToneChoice::kCustom;
  const clip::Plan custom = clip::plan_sound({line("1", hmi::Severity::kHigh, "solar")}, 2, by_source);   // a chosen tone sounds whatever the floor says
  CHECK(custom.clip.has_value() && custom.clip->kind == clip::Kind::kCustom && custom.cue == hmi::Cue::kAlarm);
  hmi::AlarmLine odd = line("1", hmi::Severity::kHigh, "solar");
  odd.code = "Bad Code&x";
  const clip::Plan odd_plan = clip::plan_sound({odd}, 0, by_source);
  CHECK(odd_plan.clip.has_value() && odd_plan.clip->code.empty());   // a code that is not plain is never put in a request
}

static void voice_edges() {
  using voice::Action;
  using voice::Event;
  using voice::State;
  voice::Limits limits;
  voice::Session off(limits);
  CHECK(off.state() == State::kOff);
  for (Event event : {Event::kWake, Event::kSpeechEnded, Event::kReply, Event::kSpeechDone, Event::kFailure, Event::kTouch}) CHECK(off.on_event(event, 10) == Action::kNone && off.state() == State::kOff);
  CHECK(off.tick(1'000'000) == Action::kNone);

  voice::Session session(limits);
  session.set_enabled(true, 0);
  CHECK(session.state() == State::kIdle);
  // events out of order do nothing
  CHECK(session.on_event(Event::kSpeechEnded, 10) == Action::kNone && session.on_event(Event::kReply, 10) == Action::kNone && session.on_event(Event::kSpeechDone, 10) == Action::kNone);
  CHECK(session.on_event(Event::kTouch, 10) == Action::kNone && session.on_event(Event::kFailure, 10) == Action::kNone && session.state() == State::kIdle);
  CHECK(session.on_event(Event::kWake, 5000) == Action::kStartRecording && session.state() == State::kListening);
  CHECK(session.on_event(Event::kWake, 5100) == Action::kNone);                       // a second wake word while listening
  CHECK(session.on_event(Event::kReply, 5200) == Action::kNone);                      // a reply nobody asked for
  // the microphone is cut at the limit
  CHECK(session.tick(5000 + limits.listen_ms - 1) == Action::kNone && session.state() == State::kListening);
  CHECK(session.tick(5000 + limits.listen_ms) == Action::kStopRecordingAndSend && session.state() == State::kThinking);
  // the speech service never answers
  CHECK(session.tick(5000 + limits.listen_ms + limits.think_ms - 1) == Action::kNone);
  CHECK(session.tick(5000 + limits.listen_ms + limits.think_ms) == Action::kShowFailure && session.state() == State::kFailed && session.failure() == "timeout");
  CHECK(session.on_event(Event::kReply, 40000) == Action::kNone && session.state() == State::kFailed);   // a late reply is dropped
  const std::uint32_t failed_at = 5000 + limits.listen_ms + limits.think_ms;
  CHECK(session.tick(failed_at + limits.failure_show_ms - 1) == Action::kNone);
  CHECK(session.tick(failed_at + limits.failure_show_ms) == Action::kBackToIdle && session.state() == State::kIdle);
  // the sound of the speaker right after the conversation is not a new wake word
  const std::uint32_t ended = failed_at + limits.failure_show_ms;
  CHECK(session.on_event(Event::kWake, ended + limits.wake_cooldown_ms - 1) == Action::kNone);
  CHECK(session.on_event(Event::kWake, ended + limits.wake_cooldown_ms) == Action::kStartRecording);

  // a touch stops anything, and a touch dismisses a failure without an audio stop
  CHECK(session.on_event(Event::kTouch, ended + 2000) == Action::kStopAudio && session.state() == State::kIdle);
  voice::Session talk(limits);
  talk.set_enabled(true, 0);
  talk.on_event(Event::kWake, 10000);
  talk.on_event(Event::kSpeechEnded, 11000);
  CHECK(talk.state() == State::kThinking && talk.on_event(Event::kReply, 12000) == Action::kStartSpeaking && talk.state() == State::kSpeaking);
  CHECK(talk.tick(12000 + limits.speak_ms - 1) == Action::kNone);
  CHECK(talk.tick(12000 + limits.speak_ms) == Action::kStopAudio && talk.state() == State::kIdle);   // a reply that never finishes does not lock the screen
  talk.on_event(Event::kWake, 100000);
  CHECK(talk.on_event(Event::kFailure, 100100, "mic") == Action::kShowFailure && talk.failure() == "mic");
  CHECK(talk.on_event(Event::kFailure, 100200, "again") == Action::kNone && talk.failure() == "mic");   // the first reason stays
  CHECK(talk.on_event(Event::kTouch, 100300) == Action::kBackToIdle && talk.state() == State::kIdle);
  talk.on_event(Event::kWake, 200000);
  CHECK(talk.on_event(Event::kFailure, 200100) == Action::kShowFailure && talk.failure() == "failed");   // a failure without a reason still says something
  // still() applies the time limits and a switch-off ends everything
  voice::Session probe(limits);
  probe.set_enabled(true, 0);
  probe.on_event(Event::kWake, 50000);
  CHECK(probe.still(State::kListening, 50000 + limits.listen_ms - 1));
  CHECK(!probe.still(State::kListening, 50000 + limits.listen_ms) && probe.state() == State::kThinking);
  probe.set_enabled(false, 60000);
  CHECK(probe.state() == State::kOff && !probe.still(State::kThinking, 60001));
  probe.set_enabled(true, 61000);
  CHECK(probe.state() == State::kIdle);
  probe.set_listen_ms(1000);
  probe.on_event(Event::kWake, 70000);
  CHECK(probe.tick(70999) == Action::kNone && probe.tick(71000) == Action::kStopRecordingAndSend);
  CHECK(std::string(voice::to_text(State::kSpeaking)) == "speaking" && std::string(voice::to_text(State::kOff)) == "off");

  // any sequence of events at any times leaves the machine in a state of the list and never makes it go back in time (fixed seed)
  for (int round = 0; round < 100; ++round) {
    voice::Session fuzz(limits);
    fuzz.set_enabled(true, 0);
    std::uint32_t now = 0;
    for (int step = 0; step < 80; ++step) {
      now += next_random() % 9000;
      const Event event = static_cast<Event>(next_random() % 6);
      fuzz.on_event(event, now, "x");
      fuzz.tick(now);
      const State state = fuzz.state();
      CHECK(state == State::kIdle || state == State::kListening || state == State::kThinking || state == State::kSpeaking || state == State::kFailed);
      // a long silence always brings the machine home
    }
    fuzz.tick(now + 1000000);
    fuzz.tick(now + 2000000);
    fuzz.tick(now + 3000000);
    CHECK(fuzz.state() == State::kIdle);
  }
}

static void stale_server_data() {
  hmi::Summary summary;
  summary.mode = hmi::Mode::kArmed;
  hmi::LinkTracker link(3);
  link.configure(true, 3);
  link.set_network(true);
  link.update(0);
  CHECK(link.link() == hmi::Link::kConnecting && !link.current() && !link.old() && !link.has_summary());
  link.on_summary(1000, summary);
  CHECK(link.link() == hmi::Link::kOnline && link.current() && link.stale_after_ms() == 4 * 3000);
  link.update(1000 + 12000);                                       // exactly four polls: not yet
  CHECK(link.current() && !link.old());
  link.update(1000 + 12001);
  CHECK(link.link() == hmi::Link::kStale && link.old() && !link.current() && link.has_summary());   // old data is shown dimmed, never as fresh
  link.on_summary(40000, summary);
  CHECK(link.current() && !link.old());

  // the age does not depend on why the answers stopped
  link.set_network(false);
  link.update(40000 + 12001);
  CHECK(link.link() == hmi::Link::kNoNetwork && link.old() && !link.current());
  link.set_network(true);
  link.on_failure(60000, hmi::Failure::kForbidden);
  CHECK(link.link() == hmi::Link::kDenied && link.old() && !link.current());
  link.on_summary(61000, summary);
  CHECK(link.link() == hmi::Link::kOnline && link.current());

  // the live channel stretches the limit to three heartbeats, never below the polls
  link.set_live_heartbeat_ms(10000);
  CHECK(link.stale_after_ms() == 30000);
  link.set_live_heartbeat_ms(1000);
  CHECK(link.stale_after_ms() == 12000);
  link.set_live_heartbeat_ms(-5);
  CHECK(link.stale_after_ms() == 12000);
  link.update(61000 + 12000);
  CHECK(link.current());

  // a refused login: two in a row mean the login itself is wrong
  hmi::LinkTracker refused(3);
  refused.configure(true, 3);
  refused.set_network(true);
  refused.on_failure(1000, hmi::Failure::kUnauthorized);
  CHECK(refused.link() != hmi::Link::kDenied && !refused.signed_in());
  refused.on_failure(2000, hmi::Failure::kUnauthorized);
  CHECK(refused.link() == hmi::Link::kDenied);
  refused.on_login_accepted();
  refused.on_failure(3000, hmi::Failure::kUnauthorized);
  CHECK(refused.link() == hmi::Link::kDenied || refused.failures() == 3);

  // the pace of the retries doubles to a minute and no further
  hmi::LinkTracker pace(3);
  pace.configure(true, 3);
  pace.set_network(true);
  CHECK(pace.next_delay_ms() == 3000);
  std::int64_t last = 3000;
  for (int i = 0; i < 12; ++i) {
    pace.on_failure(1000 + i, hmi::Failure::kNoReply);
    const std::int64_t delay = pace.next_delay_ms();
    CHECK(delay >= last && delay <= 60000);
    last = delay;
  }
  CHECK(last == 60000);
  pace.on_summary(5000, summary);
  CHECK(pace.next_delay_ms() == 3000 && pace.failures() == 0);
  pace.configure(false, 3);
  CHECK(pace.link() == hmi::Link::kNotConfigured && !pace.current());
  hmi::LinkTracker quick(0);   // a poll of zero seconds is one second
  quick.configure(true, 0);
  CHECK(quick.next_delay_ms() == 1000 && quick.stale_after_ms() == 4000);

  // an alarm that drops out of the summary and comes back does not ring again; the first summary after a start rings for nothing
  hmi::Summary now;
  now.alarms.push_back(line("a", hmi::Severity::kHigh));
  now.alarms.push_back(line("b", hmi::Severity::kCritical));
  CHECK(hmi::new_unacknowledged({}, true, now).empty());
  std::vector<std::string> known;
  CHECK(hmi::new_unacknowledged(known, false, now).size() == 2);
  hmi::remember_ids(known, now);
  CHECK(hmi::new_unacknowledged(known, false, now).empty());
  now.alarms[0].acknowledged = true;
  CHECK(hmi::tone_of(now) == hmi::Tone::kAlert);
  now.alarms[1].acknowledged = true;
  CHECK(hmi::tone_of(now) == hmi::Tone::kCalm);
  now.alarms_unacknowledged = 9;                                    // more than the six that were sent
  CHECK(hmi::tone_of(now) == hmi::Tone::kReview);
  for (int i = 0; i < 100; ++i) {
    hmi::Summary more;
    more.alarms.push_back(line(("n" + std::to_string(i)).c_str(), hmi::Severity::kInfo));
    hmi::remember_ids(known, more, 16);
  }
  CHECK(known.size() == 16 && known.back() == "n99");

  // a summary that is cut, wrong or hostile is refused and leaves what the panel had
  hmi::Summary keep;
  keep.nodes_online = 42;
  CHECK(!hmi::parse_summary("", keep) && !hmi::parse_summary("{}", keep) && !hmi::parse_summary("[]", keep));
  CHECK(!hmi::parse_summary("{\"mode\":\"other\",\"nodes\":{},\"alarms\":{}}", keep));
  CHECK(!hmi::parse_summary("{\"mode\":\"armed\"}", keep));
  CHECK(!hmi::parse_summary("{\"mode\":\"armed\",\"nodes\":1,\"alarms\":{}}", keep));
  CHECK(keep.nodes_online == 42);
  const std::string whole = "{\"mode\":\"armed\",\"revision\":3,\"nodes\":{\"online\":2,\"total\":5},\"alarms\":{\"active\":1,\"unacknowledged\":1,\"items\":[{\"id\":\"x\",\"code\":\"duress_disarm\",\"severity\":\"critical\"}]}}";
  hmi::Summary parsed;
  CHECK(hmi::parse_summary(whole, parsed) && parsed.alarms.empty() && parsed.alarms_active == 0 && parsed.alarms_unacknowledged == 0);   // the duress alarm is never shown nor counted
  for (std::size_t cut = 0; cut < whole.size(); ++cut) {
    hmi::Summary partial;
    CHECK(!hmi::parse_summary(whole.substr(0, cut), partial));
  }
  CHECK(hmi::power_text(0) == "0 W" && hmi::power_text(999) == "999 W" && hmi::power_text(1000) == "1.0 kW" && hmi::power_text(-1500) == "-1.5 kW" && hmi::power_text(12000) == "12 kW");
}

int main() {
  pin_pad();
  confirmation_expiry();
  pin_refusals();
  alarm_audio();
  voice_edges();
  stale_server_data();
  FINISH("screens_edges");
}
