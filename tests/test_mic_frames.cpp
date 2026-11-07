// ARMOR-NODE-HMI - host tests of the microphone's samples out of a four-slot frame.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <cstdint>
#include <vector>

#include "../core/mic_frames.hpp"
#include "check.hpp"

using namespace armor::mic;

static void test_each_slot_is_its_own_microphone() {
  // frame n holds 1000*slot + n, so a mixed-up slot shows at once
  std::vector<std::int16_t> stream;
  for (int n = 0; n < 6; ++n) for (int slot = 0; slot < 4; ++slot) stream.push_back(static_cast<std::int16_t>(1000 * slot + n));
  for (std::size_t slot = 0; slot < kSlots; ++slot) {
    std::int16_t out[6] = {};
    CHECK(extract_slot(stream.data(), 6, slot, out) == 6);
    for (int n = 0; n < 6; ++n) CHECK(out[n] == 1000 * static_cast<int>(slot) + n);
  }
}

static void test_the_listened_slot_is_the_first_and_never_mixes_two() {
  CHECK(kSlots == 4 && kUsedSlot == 0);
  std::vector<std::int16_t> stream;
  for (int n = 0; n < 320; ++n) for (int slot = 0; slot < 4; ++slot) stream.push_back(static_cast<std::int16_t>(slot == 0 ? n : -1 - slot));
  std::vector<std::int16_t> out(320);
  CHECK(extract_slot(stream.data(), 320, kUsedSlot, out.data()) == 320);
  bool clean = true;
  for (int n = 0; n < 320; ++n) if (out[static_cast<std::size_t>(n)] != n) clean = false;   // no sample of another microphone
  CHECK(clean);
}

static void test_bad_arguments_write_nothing() {
  std::int16_t in[8] = {1, 2, 3, 4, 5, 6, 7, 8}, out[2] = {99, 99};
  CHECK(extract_slot(nullptr, 2, 0, out) == 0 && extract_slot(in, 2, 0, nullptr) == 0);
  CHECK(extract_slot(in, 2, 4, out) == 0 && out[0] == 99 && out[1] == 99);   // a slot the line does not have
  CHECK(extract_slot(in, 0, 0, out) == 0 && out[0] == 99);                   // no frames
  CHECK(extract_slot(in, 2, 3, out) == 2 && out[0] == 4 && out[1] == 8);
}

int main() {
  test_each_slot_is_its_own_microphone();
  test_the_listened_slot_is_the_first_and_never_mixes_two();
  test_bad_arguments_write_nothing();
  FINISH("test_mic_frames");
}
