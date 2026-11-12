// ARMOR-NODE-HMI - host tests of the panel's side of the wake word: the chunks and the gate.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <cstdint>
#include <vector>

#include "../core/wake_stream.hpp"
#include "check.hpp"

using namespace armor::wake;

static void test_a_chunk_is_cut_to_the_length_asked_and_never_beyond_what_the_server_takes() {
  CHECK(Chunker(1000).samples_per_chunk() == 16000);
  CHECK(Chunker(10).samples_per_chunk() == 16000 * kMinMs / 1000);   // too short is raised
  CHECK(Chunker(60000).samples_per_chunk() == kMaxSamples);          // too long is lowered to two seconds
  CHECK(Chunker().samples_per_chunk() == 16000 * kDefaultMs / 1000);
  CHECK(clamp_ms(0) == kMinMs && clamp_ms(5000) == kMaxMs && clamp_ms(500) == 500);
}

static void test_samples_fill_a_chunk_and_the_rest_is_offered_again() {
  Chunker chunker(240);   // 3840 samples
  std::vector<std::int16_t> input(5000, 7);
  CHECK(!chunker.ready());
  CHECK(chunker.push(input.data(), input.size()) == 3840);
  CHECK(chunker.ready() && chunker.bytes() == 3840 * 2);
  CHECK(chunker.push(input.data(), 10) == 0);   // full: nothing more until it is taken
  CHECK(chunker.data()[0] == 7 && chunker.data()[3839] == 7);
  chunker.take();
  CHECK(!chunker.ready() && chunker.bytes() == 0);
  CHECK(chunker.push(input.data() + 3840, 1160) == 1160);
  CHECK(!chunker.ready());
}

static void test_no_pointer_and_no_samples_are_nothing() {
  Chunker chunker;
  CHECK(chunker.push(nullptr, 100) == 0);
  std::int16_t one = 1;
  CHECK(chunker.push(&one, 0) == 0);
  CHECK(chunker.bytes() == 0);
}

static void test_it_is_off_until_the_setting_turns_it_on() {
  Gate gate;
  CHECK(!gate.may_send(0) && !gate.may_send(100000));
  gate.set_enabled(true);
  CHECK(gate.may_send(0));
  gate.set_enabled(false);
  CHECK(!gate.may_send(5));
}

static void test_nothing_is_sent_while_the_panel_speaks_or_records() {
  Gate gate;
  gate.set_enabled(true);
  gate.set_speaking(true);
  CHECK(!gate.may_send(10));
  gate.set_speaking(false);
  gate.set_recording(true);
  CHECK(!gate.may_send(10));
  gate.set_recording(false);
  CHECK(gate.may_send(10));
}

static void test_a_server_that_does_not_answer_is_asked_less_and_less_often() {
  Gate gate;
  gate.set_enabled(true);
  std::uint32_t now = 1000;
  std::uint32_t expected[] = {1000, 2000, 4000, 8000, 16000, 32000, 32000, 32000};
  for (std::uint32_t wait : expected) {
    gate.result(false, false, now);
    CHECK(!gate.may_send(now + wait - 1));
    CHECK(gate.may_send(now + wait));
    now += wait;
  }
  CHECK(gate.failures() == 6);
  gate.result(true, false, now);
  CHECK(gate.failures() == 0 && gate.may_send(now));
}

static void test_after_a_wake_the_panel_is_left_alone_for_a_while_and_a_late_clock_wrap_is_safe() {
  Gate gate;
  gate.set_enabled(true);
  gate.result(true, true, 10000);
  CHECK(!gate.may_send(10000) && !gate.may_send(12999));
  CHECK(gate.may_send(13000));
  Gate wrapping;
  wrapping.set_enabled(true);
  wrapping.result(true, true, 0xFFFFFF00u);   // the clock of 32 bits goes round while it waits
  CHECK(!wrapping.may_send(0xFFFFFF10u));
  CHECK(wrapping.may_send(0xFFFFFF00u + Gate::kAfterWakeMs));
  CHECK(wrapping.may_send(3000u));
}

static void test_the_answer_of_the_service_is_read_strictly() {
  bool woke = true;
  CHECK(parse_answer("{\"wake\":false}", woke) && !woke);
  CHECK(parse_answer(" {\"wake\": true, \"score\": 0.9} ", woke) && woke);
  woke = true;
  CHECK(!parse_answer("", woke) && woke);                       // a refusal leaves the value alone
  CHECK(!parse_answer("{\"wake\":\"yes\"}", woke));            // not a boolean
  CHECK(!parse_answer("{\"wake\":1}", woke));
  CHECK(!parse_answer("{\"error\":\"wake_disabled\"}", woke));  // the service answers 403 with this
  CHECK(!parse_answer("[true]", woke) && !parse_answer("true", woke) && !parse_answer("{\"wake\":tr", woke));
  CHECK(!parse_answer(std::string(300, ' ') + "{\"wake\":true}", woke));   // longer than any answer of the service
}

int main() {
  test_a_chunk_is_cut_to_the_length_asked_and_never_beyond_what_the_server_takes();
  test_samples_fill_a_chunk_and_the_rest_is_offered_again();
  test_no_pointer_and_no_samples_are_nothing();
  test_it_is_off_until_the_setting_turns_it_on();
  test_nothing_is_sent_while_the_panel_speaks_or_records();
  test_a_server_that_does_not_answer_is_asked_less_and_less_often();
  test_after_a_wake_the_panel_is_left_alone_for_a_while_and_a_late_clock_wrap_is_safe();
  test_the_answer_of_the_service_is_read_strictly();
  FINISH("test_wake_stream");
}
