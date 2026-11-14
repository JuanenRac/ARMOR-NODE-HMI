// ARMOR-NODE-HMI - the camera picture on demand: what the panel accepts of a still picture and when it asks for the next one. Pure: tested on a computer.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// The panel shows one camera, chosen in its settings (camera.id), only while the person has the picture open and only if the server lets its account read it (the option
// panel.camera_view; off by default). It asks for a JPEG every few seconds (camera.refresh_s, 2 to 60), never faster than the server allows, and keeps nothing: the picture lives in memory
// until the next one replaces it or the person closes it.
//
// What the panel accepts: a baseline JPEG, at most kStillMaxBytes long, whose size in pixels fits the memory kept for decoding it. A picture that is cut, that is not a JPEG, that is
// progressive (the small decoder of the screen does not read it) or too large is refused with a short code, and the screen says there is no picture.
#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace armor::hmi {

constexpr std::size_t kStillMaxBytes = 1024 * 1024;           // the server never sends more (it answers 502 over 1 MiB)
constexpr std::uint32_t kStillMaxPixels = 640u * 480u;         // decoded at two bytes a pixel it takes at most 600 KB of the external RAM
constexpr int kStillMinRefreshS = 2;
constexpr int kStillMaxRefreshS = 60;
constexpr int kStillDefaultRefreshS = 5;

enum class StillCheck { kOk, kNotJpeg, kTruncated, kTooBig, kProgressive, kTooManyPixels, kNoSize };

inline const char* still_check_name(StillCheck check) {
  switch (check) {
    case StillCheck::kOk: return "ok";
    case StillCheck::kNotJpeg: return "not_jpeg";
    case StillCheck::kTruncated: return "truncated";
    case StillCheck::kTooBig: return "too_big";
    case StillCheck::kProgressive: return "progressive";
    case StillCheck::kTooManyPixels: return "too_many_pixels";
    case StillCheck::kNoSize: return "no_size";
  }
  return "not_jpeg";
}

inline int clamp_refresh_s(int seconds) {
  return seconds < kStillMinRefreshS ? kStillMinRefreshS : (seconds > kStillMaxRefreshS ? kStillMaxRefreshS : seconds);
}

// Looks at the JPEG without decoding it: the start and end marks, the kind of frame and the size in pixels. `width` and `height` are set when the size was read.
inline StillCheck check_still(std::string_view data, std::uint32_t& width, std::uint32_t& height) {
  width = height = 0;
  if (data.size() > kStillMaxBytes) return StillCheck::kTooBig;
  const auto byte = [&](std::size_t at) { return static_cast<std::uint8_t>(data[at]); };
  if (data.size() < 4 || byte(0) != 0xFF || byte(1) != 0xD8) return StillCheck::kNotJpeg;
  if (byte(data.size() - 2) != 0xFF || byte(data.size() - 1) != 0xD9) return StillCheck::kTruncated;
  std::size_t at = 2;
  while (at + 4 <= data.size()) {
    if (byte(at) != 0xFF) return StillCheck::kNotJpeg;
    const std::uint8_t marker = byte(at + 1);
    if (marker == 0xFF) { ++at; continue; }                                // padding
    if (marker == 0xD8 || marker == 0x01 || (marker >= 0xD0 && marker <= 0xD7)) { at += 2; continue; }   // marks with no length
    if (marker == 0xD9 || marker == 0xDA) break;                            // the end, or the data of the scan: no frame header came before
    const std::size_t length = (static_cast<std::size_t>(byte(at + 2)) << 8) | byte(at + 3);
    if (length < 2 || at + 2 + length > data.size()) return StillCheck::kTruncated;
    const bool frame = marker >= 0xC0 && marker <= 0xCF && marker != 0xC4 && marker != 0xC8 && marker != 0xCC;
    if (frame) {
      if (marker != 0xC0 && marker != 0xC1) return StillCheck::kProgressive;   // only the baseline kinds are read by the screen's decoder
      if (length < 8) return StillCheck::kTruncated;
      height = (static_cast<std::uint32_t>(byte(at + 5)) << 8) | byte(at + 6);
      width = (static_cast<std::uint32_t>(byte(at + 7)) << 8) | byte(at + 8);
      if (width == 0 || height == 0) return StillCheck::kNoSize;
      return static_cast<std::uint64_t>(width) * height > kStillMaxPixels ? StillCheck::kTooManyPixels : StillCheck::kOk;
    }
    at += 2 + length;
  }
  return StillCheck::kNoSize;
}

// When the next picture is asked for. A refused or failed ask waits longer each time (up to a minute), a 429 waits what the server said, and a good picture goes back to the normal pace.
class StillSchedule {
 public:
  void configure(int refresh_s) { refresh_s_ = clamp_refresh_s(refresh_s); }
  void opened(std::int64_t now_ms) { next_ms_ = now_ms; failures_ = 0; }   // the person opened the picture: the first one is asked for now
  bool due(std::int64_t now_ms) const { return now_ms >= next_ms_; }
  void succeeded(std::int64_t now_ms) { failures_ = 0; next_ms_ = now_ms + static_cast<std::int64_t>(refresh_s_) * 1000; }
  void failed(std::int64_t now_ms, int retry_after_s = 0) {
    if (failures_ < 6) ++failures_;
    std::int64_t wait = static_cast<std::int64_t>(refresh_s_) * 1000 * (1 << (failures_ - 1));
    if (wait > 60000) wait = 60000;
    const std::int64_t asked = static_cast<std::int64_t>(retry_after_s < 0 ? 0 : (retry_after_s > 60 ? 60 : retry_after_s)) * 1000;
    next_ms_ = now_ms + (asked > wait ? asked : wait);
  }
  int failures() const { return failures_; }
  int refresh_s() const { return refresh_s_; }

 private:
  int refresh_s_ = kStillDefaultRefreshS;
  int failures_ = 0;
  std::int64_t next_ms_ = 0;
};

}  // namespace armor::hmi
