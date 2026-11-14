// ARMOR-NODE-HMI - the check before arming, as the panel shows it: what the server found that would leave the house blind or noisy (a node that does not answer, a door that is open...),
// in the few lines the confirmation has room for. Pure: tested on a computer.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// The server answers GET /api/v1/preflight with { ready, checked_at, blocking, warnings, issues: [{ kind, level, code, id, detail? }] }. The panel asks for it only when the setting
// controls.preflight is on (off by default) and the server's option panel.preflight lets its account read it; if the answer does not come, is refused or is not what the server sends, the
// panel arms exactly as it did before: the check informs, it never stops the person (the server arms whatever the check found, as it does for Studio and the phone).
#pragma once
#include <array>
#include <cstddef>
#include <cstring>
#include <string>
#include <string_view>

#include "json.hpp"

namespace armor::hmi {

constexpr std::size_t kPreflightRows = 5;       // as many findings as the confirmation has room for; the others are counted
constexpr std::size_t kPreflightCodeBytes = 20;
constexpr std::size_t kPreflightIdBytes = 40;

struct PreflightRow {
  char code[kPreflightCodeBytes] = {};
  char id[kPreflightIdBytes] = {};
  bool block = false;
};

struct Preflight {
  bool valid = false;           // the answer was what the server sends
  bool ready = false;           // nothing blocks
  int blocking = 0, warnings = 0;
  std::size_t row_count = 0;
  std::array<PreflightRow, kPreflightRows> rows{};
  int more = 0;                 // findings that did not fit
};

// The key of the screen text for a finding code; an unknown code (a newer server) has a general one.
inline const char* preflight_key(std::string_view code) {
  if (code == "node_offline") return "pf_node_offline";
  if (code == "panel_silent") return "pf_panel_silent";
  if (code == "zone_open") return "pf_zone_open";
  if (code == "camera_down") return "pf_camera_down";
  if (code == "opening_open") return "pf_opening_open";
  if (code == "device_offline") return "pf_device_offline";
  if (code == "alarm_unseen") return "pf_alarm_unseen";
  return "pf_other";
}

template <std::size_t N>
inline void preflight_copy(char (&field)[N], std::string_view text) {
  const std::size_t n = text.size() < N - 1 ? text.size() : N - 1;
  std::memcpy(field, text.data(), n);
  std::memset(field + n, 0, N - n);
}

// The answer of the server as the block the screen draws: the findings that block come first, then the warnings, at most kPreflightRows of them. False when it is not the server's answer.
inline bool preflight_from_value(const json::Value& document, Preflight& out) {
  if (!document.is_object()) return false;
  const json::Value* ready = document.get("ready");
  const json::Value* issues = document.get("issues");
  if (ready == nullptr || !ready->is_bool() || issues == nullptr || !issues->is_array()) return false;
  Preflight result;
  result.valid = true;
  result.ready = ready->boolean;
  int total = 0;
  for (int pass = 0; pass < 2; ++pass) {   // pass 0: the findings that block; pass 1: the warnings
    for (const json::Value& item : issues->items) {
      if (!item.is_object()) continue;
      const std::string code = item.string_or("code", "");
      if (code.empty()) continue;
      const bool block = item.string_or("level", "warn") == "block";
      if (block != (pass == 0)) continue;
      ++total;
      if (block) ++result.blocking; else ++result.warnings;
      if (result.row_count < kPreflightRows) {
        PreflightRow& row = result.rows[result.row_count++];
        preflight_copy(row.code, code);
        preflight_copy(row.id, item.string_or("id", ""));
        row.block = block;
      }
    }
  }
  result.more = total > static_cast<int>(result.row_count) ? total - static_cast<int>(result.row_count) : 0;
  result.ready = result.ready && result.blocking == 0;   // a server that says ready with a blocking finding in the list is not believed
  out = result;
  return true;
}

inline bool parse_preflight(std::string_view text, Preflight& out) {
  json::Value document;
  if (!json::parse(text, document)) return false;
  return preflight_from_value(document, out);
}

// What the confirmation to arm shows about the check: nothing (the setting is off, or the check could not be read), that it is being read, that all is well, or the findings.
enum class PreflightShown { kNothing, kChecking, kAllClear, kFindings };

inline PreflightShown preflight_shown(bool setting_on, bool asking, const Preflight& result) {
  if (!setting_on) return PreflightShown::kNothing;
  if (asking) return PreflightShown::kChecking;
  if (!result.valid) return PreflightShown::kNothing;
  return result.row_count == 0 && result.more == 0 ? PreflightShown::kAllClear : PreflightShown::kFindings;
}

}  // namespace armor::hmi
