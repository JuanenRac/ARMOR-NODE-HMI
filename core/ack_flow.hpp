// ARMOR-NODE-HMI - acknowledging the alarms from the wall panel: what a touch on "Acknowledge all" asks, what is sent to the server and what the server's answer means. Pure: tested on a computer.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// Anyone who can touch the panel can acknowledge, and acknowledging a critical alarm stops the server from going on warning people. So the panel asks first, the way it asks before it disarms:
//   pin      the keypad: the PIN of the panel's account is sent with the order and the server checks it (a wrong one is refused and nothing is acknowledged; the right one counts as a person
//            standing there, so the escalation of a critical alarm stops; the duress PIN acknowledges without that). This is the default.
//   confirm  a question with yes and no, no PIN: the server takes the order as it always did from a panel (unverified, a critical alarm keeps being escalated).
//   off      one touch, as before.
// The order itself is POST /api/v1/alarms/acknowledge with {} or {"pin": "..."} (ARMOR-COMMON/openapi, PostAlarmsAcknowledgeRequest); the answer is 200, 403 (wrong_pin, forbidden) or 429
// (pin_locked). The same words come back for the order that arms or disarms, so one function says what any answer means.
#pragma once
#include <string>

#include "pin_entry.hpp"

namespace armor::hmi {

enum class AckMode { kPin, kConfirm, kOff };

inline const char* ack_mode_text(AckMode mode) { return mode == AckMode::kConfirm ? "confirm" : mode == AckMode::kOff ? "off" : "pin"; }
inline bool parse_ack_mode(const std::string& text, AckMode& out) {
  for (const AckMode mode : {AckMode::kPin, AckMode::kConfirm, AckMode::kOff}) if (text == ack_mode_text(mode)) { out = mode; return true; }
  return false;
}

// What touching "Acknowledge all" does first.
enum class AckStep { kSendNow, kAskToConfirm, kAskForPin };
inline AckStep ack_step(AckMode mode) { return mode == AckMode::kPin ? AckStep::kAskForPin : mode == AckMode::kConfirm ? AckStep::kAskToConfirm : AckStep::kSendNow; }

// The body of the order. Only digits go in it: whatever else was typed is not a PIN and is dropped; no digits at all is an order without a PIN ("{}").
inline std::string ack_body(const std::string& pin) {
  std::string digits;
  for (const char c : pin) if (c >= '0' && c <= '9') digits.push_back(c);
  if (digits.empty()) return "{}";
  return "{\"pin\":\"" + digits + "\"}";
}

// The short word for the footer of the screen for the answer of the server to an order; "" when the order went through.
//   wrong_pin, pin_locked, pin_not_set   the PIN (the codes of the server, see pin_error_of)
//   no_answer                            the server did not answer (status 0)
//   unauthorized                         the session ended (401) and signing in again did not help
//   forbidden                            the server refused the order (403 for another reason)
//   failed                               anything else
inline std::string order_error(int status, const std::string& body) {
  if (status == 200) return "";
  const std::string pin = pin_error_of(status, body);
  if (!pin.empty()) return pin;
  if (status == 0) return "no_answer";
  if (status == 401) return "unauthorized";
  if (status == 403) return "forbidden";
  return "failed";
}

}  // namespace armor::hmi
