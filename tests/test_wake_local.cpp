// ARMOR-NODE-HMI - host tests of the wake word heard on the panel itself: the trigger, the choice of source and the listener, against a simulated detector.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <cmath>
#include <cstdint>
#include <vector>

#include "../core/wake_local.hpp"
#include "check.hpp"

using namespace armor::wake_local;

static std::vector<std::int16_t> silence(std::size_t n) { return std::vector<std::int16_t>(n, 0); }
static std::vector<std::int16_t> loud(std::size_t n) { return std::vector<std::int16_t>(n, 12000); }

static void test_the_trigger_needs_the_score_to_hold_for_several_frames() {
  Trigger trigger(Trigger::Settings{0.5f, 3, 1000});
  CHECK(!trigger.update(0.9f, 0));
  CHECK(!trigger.update(0.9f, 80));
  CHECK(trigger.update(0.9f, 160));   // the third frame in a row
  Trigger broken(Trigger::Settings{0.5f, 3, 1000});
  CHECK(!broken.update(0.9f, 0) && !broken.update(0.9f, 80));
  CHECK(!broken.update(0.1f, 160));   // a quiet frame starts the count again
  CHECK(!broken.update(0.9f, 240) && !broken.update(0.9f, 320));
  CHECK(broken.update(0.9f, 400));
}

static void test_after_a_wake_the_panel_is_left_alone_and_a_wrapping_clock_is_safe() {
  Trigger trigger(Trigger::Settings{0.5f, 1, 2000});
  CHECK(trigger.update(0.9f, 1000));
  CHECK(!trigger.update(0.9f, 1500) && !trigger.update(0.9f, 2999));   // inside the quiet time
  CHECK(trigger.update(0.9f, 3000));                                   // the quiet time is over
  Trigger wrap(Trigger::Settings{0.5f, 1, 2000});
  CHECK(wrap.update(0.9f, 0xFFFFFF00u));
  CHECK(!wrap.update(0.9f, 0xFFFFFF80u));
  CHECK(wrap.update(0.9f, 0x00000800u));   // the clock went past zero
}

static void test_the_tuned_values_are_held_to_their_ranges() {
  const Trigger::Settings low = Trigger::clamped(Trigger::Settings{-1.0f, 0, 0});
  CHECK(low.threshold >= 0.05f && low.frames == 1 && low.refractory_ms == 500);
  const Trigger::Settings high = Trigger::clamped(Trigger::Settings{5.0f, 99, 999999});
  CHECK(high.threshold <= 0.99f && high.frames == 10 && high.refractory_ms == 30000);
  const Trigger::Settings nan = Trigger::clamped(Trigger::Settings{std::nanf(""), 2, 3000});
  CHECK(nan.threshold == 0.05f);   // not a number is the most careful reading: it is never a way to wake with nothing
  CHECK(Trigger().settings().frames == 2 && Trigger().settings().refractory_ms == 3000);
}

static void test_the_local_detector_is_used_only_when_asked_for_and_ready() {
  SimulatedDetector ready(true), missing(false);
  CHECK(choose_source(true, &ready, false) == Source::kLocal);
  CHECK(choose_source(true, &ready, true) == Source::kLocal);
  CHECK(choose_source(false, &ready, false) == Source::kNone);       // off by default
  CHECK(choose_source(false, &ready, true) == Source::kService);     // the service way is untouched
  CHECK(choose_source(true, &missing, true) == Source::kService);    // no model on the board: back to the service
  CHECK(choose_source(true, nullptr, true) == Source::kService);     // no detector in the firmware
  CHECK(choose_source(true, &missing, false) == Source::kNone);
  CHECK(choose_source(true, nullptr, false) == Source::kNone);
}

static void test_the_listener_cuts_the_sound_into_frames_and_wakes_on_a_loud_stretch() {
  SimulatedDetector detector;
  Listener listener(&detector, Trigger(Trigger::Settings{0.5f, 2, 1000}));
  CHECK(listener.available());
  const std::vector<std::int16_t> quiet = silence(kFrameSamples * 3);
  CHECK(!listener.feed(quiet.data(), quiet.size(), 0));
  CHECK(detector.frames_scored() == 3);
  const std::vector<std::int16_t> word = loud(kFrameSamples * 2);
  CHECK(listener.feed(word.data(), word.size(), 300));   // two loud frames in a row
  // a stretch that is not a whole number of frames keeps its remainder for the next call
  SimulatedDetector other;
  Listener split(&other, Trigger(Trigger::Settings{0.5f, 1, 1000}));
  const std::vector<std::int16_t> piece = loud(kFrameSamples / 2);
  CHECK(!split.feed(piece.data(), piece.size(), 0));
  CHECK(split.feed(piece.data(), piece.size(), 10));
  CHECK(other.frames_scored() == 1);
}

static void test_a_scripted_detector_plays_its_scores_and_nothing_is_heard_without_a_model() {
  SimulatedDetector scripted;
  scripted.script({0.1f, 0.9f, 0.9f});
  Listener listener(&scripted, Trigger(Trigger::Settings{0.5f, 2, 1000}));
  const std::vector<std::int16_t> sound = silence(kFrameSamples * 3);
  CHECK(listener.feed(sound.data(), sound.size(), 0));
  SimulatedDetector missing(false);
  Listener deaf(&missing, Trigger());
  CHECK(!deaf.available());
  const std::vector<std::int16_t> shout = loud(kFrameSamples * 5);
  CHECK(!deaf.feed(shout.data(), shout.size(), 0) && missing.frames_scored() == 0);   // not ready: no frame is even scored
  Listener none(nullptr, Trigger());
  CHECK(!none.available() && !none.feed(shout.data(), shout.size(), 0));
  CHECK(!listener.feed(nullptr, 100, 5000));
}

static void test_a_reset_forgets_the_partial_frame_and_the_count() {
  SimulatedDetector detector;
  Listener listener(&detector, Trigger(Trigger::Settings{0.5f, 2, 1000}));
  const std::vector<std::int16_t> one = loud(kFrameSamples);
  CHECK(!listener.feed(one.data(), one.size(), 0));   // one loud frame of two
  listener.reset();
  CHECK(!listener.feed(one.data(), one.size(), 100)); // the count started again
  CHECK(listener.feed(one.data(), one.size(), 200));
}

int main() {
  test_the_trigger_needs_the_score_to_hold_for_several_frames();
  test_after_a_wake_the_panel_is_left_alone_and_a_wrapping_clock_is_safe();
  test_the_tuned_values_are_held_to_their_ranges();
  test_the_local_detector_is_used_only_when_asked_for_and_ready();
  test_the_listener_cuts_the_sound_into_frames_and_wakes_on_a_loud_stretch();
  test_a_scripted_detector_plays_its_scores_and_nothing_is_heard_without_a_model();
  test_a_reset_forgets_the_partial_frame_and_the_count();
  FINISH("test_wake_local");
}
