// ARMOR-NODE-HMI - the audio the server makes for an alarm: the alarm said out loud, or the sound an administrator uploaded for its origin. What to ask for, how to ask, and above all
// how to check what comes back before it is played: its size, that it is whole 16-bit samples and that its SHA-256 is the one the server said. Nothing here touches the network or the
// speaker (main/server_link.cpp downloads, main/audio.cpp plays), so all of it is tested on a computer.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// The two requests (ARMOR-SERVER docs/INTEGRATION.md, "The sound of an alarm"), both answered with raw 16 kHz 16-bit mono PCM and `X-Armor-Audio-Sha256`:
//   GET /api/v1/alarm-audio/spoken?language=&source=&severity=&code=     (the server keeps the option alarm_audio.spoken; off: 404)
//   GET /api/v1/alarm-audio/sounds/<source>                              (the server keeps the option alarm_audio.custom_sounds; off or none: 404)
// The panel never plays what it cannot check, and never waits for it to play the tone of the alarm: if the audio is missing, wrong or late, the alarm sounds as its severity says.
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <list>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "audio_queue.hpp"
#include "server_view.hpp"
#include "sha256.hpp"

namespace armor::clip {

enum class Kind { kSpoken, kCustom };

struct Request {
  Kind kind = Kind::kSpoken;
  int source = 0;                                   // hmi::source_kind
  hmi::Severity severity = hmi::Severity::kWarning;
  std::string code;                                 // the alarm code ("" when it is not a plain one)
};

constexpr std::size_t kMinBytes = 3200;             // a tenth of a second
constexpr std::size_t kMaxBytesCeiling = 262144;    // whatever the settings say
constexpr std::size_t kDefaultMaxBytes = 200000;

inline const char* severity_name(hmi::Severity severity) {
  switch (severity) {
    case hmi::Severity::kCritical: return "critical";
    case hmi::Severity::kHigh: return "high";
    case hmi::Severity::kInfo: return "info";
    default: return "warning";
  }
}

// An alarm code is lower-case letters, digits and "_.:-" (at most 64): anything else is not asked about, so nothing a device sent can end up in the address of a request.
inline std::string plain_code(std::string_view code) {
  if (code.empty() || code.size() > 64) return "";
  for (std::size_t i = 0; i < code.size(); ++i) {
    const char c = code[i];
    const bool word = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
    if (!(word || (i > 0 && (c == '_' || c == '.' || c == ':' || c == '-')))) return "";
  }
  return std::string(code);
}

// The language as the settings hold it, if it is one of the eight (the server answers 422 to any other).
inline std::string plain_language(std::string_view language) {
  for (const char* known : {"en", "es", "de", "fr", "it", "ja", "zh", "pt"}) if (language == known) return std::string(language);
  return "en";
}

// The address to ask for (with the query), all of it from closed lists or checked characters.
inline std::string path_for(const Request& request, std::string_view language) {
  if (request.kind == Kind::kCustom) return std::string("/api/v1/alarm-audio/sounds/") + hmi::source_name(request.source);
  std::string path = std::string("/api/v1/alarm-audio/spoken?language=") + plain_language(language) + "&source=" + hmi::source_name(request.source) + "&severity=" + severity_name(request.severity);
  const std::string code = plain_code(request.code);
  if (!code.empty()) path += "&code=" + code;   // the characters of a plain code need no escaping
  return path;
}

enum class Check { kOk, kEmpty, kTooSmall, kTooLarge, kOddSize, kNoHash, kBadHash, kHashMismatch };

constexpr const char* check_name(Check check) {
  switch (check) {
    case Check::kOk: return "ok";
    case Check::kEmpty: return "empty";
    case Check::kTooSmall: return "too_small";
    case Check::kTooLarge: return "too_large";
    case Check::kOddSize: return "odd_size";
    case Check::kNoHash: return "no_hash";
    case Check::kBadHash: return "bad_hash";
    case Check::kHashMismatch: return "hash_mismatch";
  }
  return "unknown";
}

inline bool is_hex_digest(std::string_view text) {
  if (text.size() != 64) return false;
  for (char c : text) if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) return false;
  return true;
}

// Whether the body is audio the panel may play: not empty, not shorter than a tenth of a second, not longer than `max_bytes` (the setting, never above the ceiling), a whole number of 16-bit
// samples, with the SHA-256 the server said in `X-Armor-Audio-Sha256` and that hash really is the hash of the body.
inline Check check(std::string_view body, std::string_view announced_hash, std::size_t max_bytes) {
  const std::size_t limit = max_bytes > kMaxBytesCeiling ? kMaxBytesCeiling : max_bytes;
  if (body.empty()) return Check::kEmpty;
  if (body.size() > limit) return Check::kTooLarge;
  if (body.size() < kMinBytes) return Check::kTooSmall;
  if (body.size() % 2 != 0) return Check::kOddSize;
  if (announced_hash.empty()) return Check::kNoHash;
  if (!is_hex_digest(announced_hash)) return Check::kBadHash;
  return sha256::hex_of(body) == announced_hash ? Check::kOk : Check::kHashMismatch;
}

// The samples of a body that passed `check`.
inline std::vector<std::int16_t> samples_of(std::string_view body) {
  std::vector<std::int16_t> out(body.size() / 2);
  for (std::size_t i = 0; i < out.size(); ++i) {
    const std::uint16_t value = static_cast<std::uint16_t>(static_cast<std::uint8_t>(body[i * 2])) | static_cast<std::uint16_t>(static_cast<std::uint16_t>(static_cast<std::uint8_t>(body[i * 2 + 1])) << 8);
    out[i] = static_cast<std::int16_t>(value);
  }
  return out;
}

// What the panel remembers of the sentences it was given (they are the same each time for the same language, origin, severity and code), so that a second alarm of the same kind is said at once.
// A few entries and a bounded number of bytes; the least recently used goes first.
class Cache {
 public:
  Cache(std::size_t max_entries = 8, std::size_t max_bytes = 600000) : max_entries_(max_entries), max_bytes_(max_bytes) {}

  const std::vector<std::int16_t>* find(const std::string& key) {
    for (auto it = items_.begin(); it != items_.end(); ++it) {
      if (it->key == key) { items_.splice(items_.begin(), items_, it); return &items_.front().samples; }
    }
    return nullptr;
  }
  void put(const std::string& key, std::vector<std::int16_t> samples) {
    for (auto it = items_.begin(); it != items_.end(); ++it) if (it->key == key) { bytes_ -= it->samples.size() * 2; items_.erase(it); break; }
    if (samples.size() * 2 > max_bytes_) return;
    bytes_ += samples.size() * 2;
    items_.push_front({key, std::move(samples)});
    while (items_.size() > max_entries_ || bytes_ > max_bytes_) { bytes_ -= items_.back().samples.size() * 2; items_.pop_back(); }
  }
  std::size_t size() const { return items_.size(); }
  std::size_t bytes() const { return bytes_; }

 private:
  struct Item { std::string key; std::vector<std::int16_t> samples; };
  std::list<Item> items_;
  std::size_t max_entries_, max_bytes_, bytes_ = 0;
};

inline std::string cache_key(const Request& request, std::string_view language) { return path_for(request, language); }

namespace detail {
constexpr int rank_of(hmi::Cue cue) { return cue == hmi::Cue::kSiren ? 3 : cue == hmi::Cue::kAlarm ? 2 : cue == hmi::Cue::kNotice ? 1 : 0; }
constexpr int severity_rank(hmi::Severity severity) { return severity == hmi::Severity::kCritical ? 3 : severity == hmi::Severity::kHigh ? 2 : severity == hmi::Severity::kWarning ? 1 : 0; }
}  // namespace detail

// What to do for the alarms that have just arrived: the tone as always (`cue`, also what plays if the audio does not) and, if the most serious alarm of an origin that was set to "spoken" or
// "custom" is at least as serious as the tone, the audio to ask for in its place. A siren for a critical alarm of another origin is never replaced by the audio of a lesser one.
struct Plan {
  hmi::Cue cue = hmi::Cue::kNone;
  std::optional<Request> clip;
};

inline Plan plan_sound(const std::vector<hmi::AlarmLine>& fresh, int floor, const std::array<hmi::ToneChoice, hmi::kSourceKinds>& by_source) {
  Plan plan;
  plan.cue = hmi::cue_for(fresh, floor, by_source);
  const hmi::AlarmLine* best = nullptr;
  for (const hmi::AlarmLine& alarm : fresh) {
    const hmi::ToneChoice choice = by_source[static_cast<std::size_t>(hmi::source_kind(alarm.source_type))];
    if (choice != hmi::ToneChoice::kSpoken && choice != hmi::ToneChoice::kCustom) continue;
    if (best == nullptr || detail::severity_rank(alarm.severity) > detail::severity_rank(best->severity)) best = &alarm;
  }
  if (best == nullptr) return plan;
  const hmi::Cue own = hmi::cue_for(std::vector<hmi::AlarmLine>{*best}, 0);
  if (detail::rank_of(own) < detail::rank_of(plan.cue)) return plan;   // a more serious alarm of another origin decides the sound
  Request request;
  request.kind = by_source[static_cast<std::size_t>(hmi::source_kind(best->source_type))] == hmi::ToneChoice::kCustom ? Kind::kCustom : Kind::kSpoken;
  request.source = hmi::source_kind(best->source_type);
  request.severity = best->severity;
  request.code = plain_code(best->code);
  plan.clip = request;
  return plan;
}

}  // namespace armor::clip
