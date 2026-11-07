// ARMOR-NODE-HMI - the panel's side of the wake word: how the microphone's samples are cut into the chunks the server is asked about, and when it may ask. Pure: tested on a computer.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// The detector itself (openWakeWord) runs on the machine of the server, never here: the panel only sends short chunks of 16 kHz audio to POST /api/v1/voice/wake and reads
// {wake: true|false}. This file is what keeps that polite: a chunk is cut to a fixed length (240 ms to 2 s, the server takes at most 2 s), nothing is sent while the panel is
// speaking or already recording a turn (it would hear itself), a server that does not answer is asked less and less often, and after a wake the panel is left alone for a while.
// The setting that turns it on (wake.enabled, off by default) and the audio task that feeds it are the firmware's part; this decides, the firmware obeys.
#pragma once
#include <cstddef>
#include <cstdint>

namespace armor::wake {

constexpr std::size_t kRate = 16000;
constexpr std::size_t kMinMs = 240;
constexpr std::size_t kMaxMs = 2000;
constexpr std::size_t kDefaultMs = 1000;
constexpr std::size_t kMaxSamples = kRate * kMaxMs / 1000;

constexpr std::size_t clamp_ms(std::size_t ms) { return ms < kMinMs ? kMinMs : (ms > kMaxMs ? kMaxMs : ms); }

// Collects samples until a chunk of `ms` milliseconds is full. The buffer is fixed (no heap): the biggest chunk the server takes.
class Chunker {
 public:
  explicit Chunker(std::size_t ms = kDefaultMs) : want_(kRate * clamp_ms(ms) / 1000) {}
  std::size_t samples_per_chunk() const { return want_; }
  // Copies up to the room left in the chunk; returns how many samples were taken (the caller offers the rest again once the chunk was taken away).
  std::size_t push(const std::int16_t* in, std::size_t n) {
    if (in == nullptr) return 0;
    const std::size_t room = want_ - fill_;
    const std::size_t take = n < room ? n : room;
    for (std::size_t i = 0; i < take; ++i) buf_[fill_ + i] = in[i];
    fill_ += take;
    return take;
  }
  bool ready() const { return fill_ == want_; }
  const std::int16_t* data() const { return buf_; }
  std::size_t bytes() const { return fill_ * sizeof(std::int16_t); }
  void take() { fill_ = 0; }   // the chunk was sent (or dropped): start the next one

 private:
  std::int16_t buf_[kMaxSamples] = {};
  std::size_t want_;
  std::size_t fill_ = 0;
};

// When the panel may send a chunk. Times are milliseconds of a monotonic clock.
class Gate {
 public:
  void set_enabled(bool on) { enabled_ = on; if (!on) failures_ = 0; }
  void set_speaking(bool on) { speaking_ = on; }     // the panel is playing a sentence
  void set_recording(bool on) { recording_ = on; }   // a turn is being recorded or answered
  bool may_send(std::uint32_t now) const {
    if (!enabled_ || speaking_ || recording_) return false;
    if (static_cast<std::int32_t>(now - quiet_until_) < 0) return false;
    return static_cast<std::int32_t>(now - retry_at_) >= 0;
  }
  // The answer of the server: `ok` false is a refusal or no answer (503, 403, a timeout), `woke` the word was said.
  void result(bool ok, bool woke, std::uint32_t now) {
    if (!ok) {
      if (failures_ < 6) ++failures_;
      std::uint32_t wait = 1000u << (failures_ - 1);   // 1 s, 2 s, 4 s ... up to 32 s
      if (wait > kMaxBackoffMs) wait = kMaxBackoffMs;
      retry_at_ = now + wait;
      return;
    }
    failures_ = 0;
    retry_at_ = now;
    if (woke) quiet_until_ = now + kAfterWakeMs;
  }
  unsigned failures() const { return failures_; }

  static constexpr std::uint32_t kMaxBackoffMs = 32000;
  static constexpr std::uint32_t kAfterWakeMs = 3000;

 private:
  bool enabled_ = false;   // off until the setting says so
  bool speaking_ = false;
  bool recording_ = false;
  unsigned failures_ = 0;
  std::uint32_t retry_at_ = 0;
  std::uint32_t quiet_until_ = 0;
};

}  // namespace armor::wake
