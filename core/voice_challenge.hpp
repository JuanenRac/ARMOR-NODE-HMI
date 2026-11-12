// ARMOR-NODE-HMI - the number of the spoken challenge: the voice service draws it and it is NOT said aloud, the panel shows it on its screen for a few seconds.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// In the mode `challenge` of the security option voice.silence_confirmation, the first "silence" is not carried out: the answer of the voice service holds a number of two to six digits
// in the field `challenge`, and only that number, said within a few seconds, is passed on. Whoever is outside the house, or a loudspeaker, cannot know it, because the sound of the
// reply does not hold it. A panel that did not show it could never be answered, so a number that is not well formed makes the whole answer bad (the panel fails closed), and a number is
// shown only for a short time and then taken away. Nothing here touches the network or the hardware: it is tested on a computer.
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace armor::voice {

inline constexpr std::size_t kChallengeMinDigits = 2;
inline constexpr std::size_t kChallengeMaxDigits = 6;

// Two to six ASCII digits and nothing else.
inline bool valid_challenge(std::string_view text) {
  if (text.size() < kChallengeMinDigits || text.size() > kChallengeMaxDigits) return false;
  for (const char c : text) if (c < '0' || c > '9') return false;
  return true;
}

// The number the panel is showing and until when. The service forgets a number after twenty seconds, so the panel shows it a little less and never longer.
class ChallengeShown {
 public:
  static constexpr std::uint32_t kShowMs = 19 * 1000;

  // A reply with a number shows it from now on; a reply without one (or with one that is not well formed) takes the old one away: the service has forgotten it as well.
  void set(std::string_view number, std::uint32_t now_ms) {
    if (valid_challenge(number)) {
      number_.assign(number);
      at_ms_ = now_ms;
    } else {
      number_.clear();
    }
  }
  void clear() { number_.clear(); }
  // The number while it is still to be shown, empty otherwise. The clock may wrap: only the difference counts.
  std::string current(std::uint32_t now_ms) const { return !number_.empty() && now_ms - at_ms_ < kShowMs ? number_ : std::string(); }

 private:
  std::string number_;
  std::uint32_t at_ms_ = 0;
};

}  // namespace armor::voice
