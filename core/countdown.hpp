// ARMOR-NODE-HMI - the countdown of the exit and entry delays, big on the screen: how many seconds are left and when the panel beeps. Pure: tested on a computer.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// The server tells, in the summary, that a countdown is running (`delay`: its phase, `exit` after arming or `entry` after a door opened while armed, and the seconds left). The panel does not
// ask again every second: it keeps the seconds it was told and the moment it was told, and counts the rest on its own clock. A new summary replaces what it keeps; a summary without a
// countdown ends it at once (the person disarmed, or the delay ran out).
//
// The beeps are a setting that is off by default (controls.countdown_beeps): one a second, two a second in the last five seconds.
#pragma once
#include <cstdint>

namespace armor::hmi {

enum class DelayKind { kNone, kExit, kEntry };

constexpr int kDelayMaxSeconds = 3600;       // a delay longer than this is not a delay a person waits for: the summary's number is cut to it
constexpr int kBeepFastFromSeconds = 5;      // from this many seconds left the beeps come twice as fast
constexpr std::int64_t kBeepSlowMs = 1000;
constexpr std::int64_t kBeepFastMs = 500;

class Countdown {
 public:
  // A summary arrived. `present` false (or a number that is not positive) ends the countdown.
  void update(bool present, bool entry, int remaining_s, std::int64_t now_ms) {
    if (!present || remaining_s <= 0) { kind_ = DelayKind::kNone; return; }
    if (remaining_s > kDelayMaxSeconds) remaining_s = kDelayMaxSeconds;
    if (kind_ == DelayKind::kNone) last_beep_ms_ = -1;   // a new countdown beeps at once
    kind_ = entry ? DelayKind::kEntry : DelayKind::kExit;
    told_s_ = remaining_s;
    told_at_ms_ = now_ms;
  }

  // The seconds left now, rounded up (the screen says 1 until the last moment); 0 when there is no countdown or it is over.
  int remaining_s(std::int64_t now_ms) const {
    if (kind_ == DelayKind::kNone) return 0;
    const std::int64_t elapsed = now_ms > told_at_ms_ ? now_ms - told_at_ms_ : 0;
    const std::int64_t left_ms = static_cast<std::int64_t>(told_s_) * 1000 - elapsed;
    return left_ms <= 0 ? 0 : static_cast<int>((left_ms + 999) / 1000);
  }
  bool active(std::int64_t now_ms) const { return remaining_s(now_ms) > 0; }
  DelayKind kind(std::int64_t now_ms) const { return active(now_ms) ? kind_ : DelayKind::kNone; }

  // True when a beep is due now (and remembers that it was made). Never when `beeps` is off or no countdown runs.
  bool beep_due(std::int64_t now_ms, bool beeps) {
    if (!beeps || !active(now_ms)) return false;
    const std::int64_t gap = remaining_s(now_ms) <= kBeepFastFromSeconds ? kBeepFastMs : kBeepSlowMs;
    if (last_beep_ms_ >= 0 && now_ms - last_beep_ms_ < gap) return false;
    last_beep_ms_ = now_ms;
    return true;
  }

 private:
  DelayKind kind_ = DelayKind::kNone;
  int told_s_ = 0;
  std::int64_t told_at_ms_ = 0;
  std::int64_t last_beep_ms_ = -1;
};

}  // namespace armor::hmi
