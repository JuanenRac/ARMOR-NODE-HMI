// ARMOR-NODE-HMI - the PIN typed on the wall panel to disarm, and what the server's refusal of a PIN says. Tested on a computer; the keypad itself (main/ui.cpp) only draws it.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

#include "json.hpp"

namespace armor::hmi {

// Up to 10 digits; anything else is ignored. The screen shows only as many dots as digits typed, never the digits.
// A confirmation left half-way (the dots and digits of a PIN on the screen) is closed after thirty seconds, and at once when the screen goes to sleep: nothing is left on a dark screen for the next person.
constexpr std::int64_t kConfirmOpenMs = 30000;
inline bool confirm_expired(std::int64_t opened_ms, std::int64_t now_ms, bool screen_asleep) { return screen_asleep || now_ms - opened_ms >= kConfirmOpenMs; }

class PinEntry {
 public:
  static constexpr std::size_t kMaxDigits = 10;
  bool add(char key) {
    if (key < '0' || key > '9' || digits_.size() >= kMaxDigits) return false;
    digits_.push_back(key);
    return true;
  }
  void erase_last() { if (!digits_.empty()) digits_.pop_back(); }
  // The digits are overwritten before they are dropped: a PIN is not left behind in memory that is freed and reused.
  void clear() { for (char& c : digits_) c = '0'; digits_.clear(); }
  bool empty() const { return digits_.empty(); }
  const std::string& digits() const { return digits_; }
  std::string masked() const { return std::string(digits_.size(), '*'); }

 private:
  std::string digits_;
};

// The codes of a refusal that is about the PIN, as the contract lists them (PinErrorCode in ARMOR-COMMON openapi/armor-server.yaml, generated/armor_error_codes.hpp: tests/test_ack_flow.cpp compares them).
inline constexpr const char* kPinErrorCodes[] = {"wrong_pin", "pin_locked", "pin_not_set"};

// The short code of a refusal that is about the PIN, or "" for any other answer. The server answers 403 or 429 with a JSON body whose "code" is one of kPinErrorCodes; the body is read as JSON
// (the spacing of the text does not matter) and only the "code" is looked at, never the words of "error".
inline std::string pin_error_of(int status, const std::string& body) {
  if (status != 403 && status != 429) return "";
  json::Value doc;
  if (!json::parse(body, doc) || !doc.is_object()) return "";
  const json::Value* code = doc.get("code");
  if (code == nullptr || !code->is_string()) return "";
  for (const char* known : kPinErrorCodes)
    if (code->text == known) return known;
  return "";
}

}  // namespace armor::hmi
