// ARMOR-NODE-HMI - the samples of one microphone out of the frames the input codec delivers. Pure: tested on a computer.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// The microphones sit on a four-slot TDM line (ES7210), so every frame of the I2S port carries four 16-bit samples, one per slot, interleaved: slot 0, slot 1, slot 2, slot 3, slot 0, ...
// A read must ask for all four channels and keep one of them. Reading two channels of a four-slot stream and taking every other sample mixes the first microphone with the third.
#pragma once
#include <cstddef>
#include <cstdint>

namespace armor::mic {

constexpr std::size_t kSlots = 4;       // the slots of the TDM line, and the channels the codec is opened with
constexpr std::size_t kUsedSlot = 0;    // the first microphone is the one that is listened to

// Copies sample `slot` of each of the `frames` frames of `interleaved` (kSlots samples per frame) to `out`. Returns how many samples were written; a slot outside the line writes none.
inline std::size_t extract_slot(const std::int16_t* interleaved, std::size_t frames, std::size_t slot, std::int16_t* out) {
  if (interleaved == nullptr || out == nullptr || slot >= kSlots) return 0;
  for (std::size_t i = 0; i < frames; ++i) out[i] = interleaved[i * kSlots + slot];
  return frames;
}

}  // namespace armor::mic
