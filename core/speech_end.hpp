// ARMOR-NODE-HMI - where a spoken request ends: the recording stops by itself when the person has stopped talking, instead of running to the longest time. Pure: tested on a computer.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// The voice assistant records after a touch and, until now, until a second touch or `voice.listen_s` seconds. A short question then left the person waiting for the rest of the time. This looks
// at the level of the sound in frames of a few milliseconds (the root mean square of the 16-bit samples) and says when the request is over:
//   - the first `calibrate_ms` are the room: the quietest frame in them is the level of the noise. If even that is louder than five times `min_level`, somebody is already talking (or the room
//     is too loud to tell): the room is taken as quiet and the request as begun, so that it still ends at the quiet after the last word, or at the longest time when the room never goes quiet;
//   - a voice is a frame louder than the noise by `level_percent` per cent, and never quieter than `min_level`;
//   - speech has started once the voice has added up to `min_speech_ms` (a click or a cough is not a question);
//   - after that, `silence_ms` without a voice ends the request. Once it has started, a frame counts as a voice at 70 % of the level that started it (and never less than a quarter above the room),
//     so the soft end of a word is not taken for silence;
//   - if no speech starts within `lead_ms` the recording stops as "no speech" (nobody asked anything).
// It is off by default (`voice.end_of_speech`): nothing here has run on a board, and a room that is louder than the voice is something only a real microphone can tell.
#pragma once
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace armor::hmi {

struct SpeechEndSettings {
  bool enabled = false;
  int silence_ms = 900;       // 300..3000: quiet that ends a request
  int min_speech_ms = 250;    // 80..1500: voice that must be heard before a request counts
  int level_percent = 300;    // 150..1000: how much louder than the room a voice is, in per cent
  int min_level = 200;        // 50..4000: the least level (RMS of the 16-bit samples) that is a voice, whatever the room
  int lead_ms = 4000;         // 1000..15000: how long to wait for the first word
  int calibrate_ms = 300;     // 100..1000: the beginning of the recording that is taken for the room
};

// "" when the settings are fine, else the name of the member that is not.
inline const char* check(const SpeechEndSettings& s) {
  if (s.silence_ms < 300 || s.silence_ms > 3000) return "silence_ms";
  if (s.min_speech_ms < 80 || s.min_speech_ms > 1500) return "min_speech_ms";
  if (s.level_percent < 150 || s.level_percent > 1000) return "level_percent";
  if (s.min_level < 50 || s.min_level > 4000) return "min_level";
  if (s.lead_ms < 1000 || s.lead_ms > 15000) return "lead_ms";
  if (s.calibrate_ms < 100 || s.calibrate_ms > 1000) return "calibrate_ms";
  if (s.calibrate_ms >= s.lead_ms) return "calibrate_ms";   // the room is measured before the wait for the first word ends
  return "";
}

enum class SpeechVerdict { kContinue, kSpeechEnded, kNoSpeech };

class SpeechEnd {
 public:
  explicit SpeechEnd(const SpeechEndSettings& settings, unsigned sample_rate_hz = 16000) : s_(settings), rate_(sample_rate_hz == 0 ? 16000 : sample_rate_hz) {}

  // The next frame of the recording (any size). The verdict is final once it is not kContinue.
  SpeechVerdict feed(const std::int16_t* samples, std::size_t count) {
    if (verdict_ != SpeechVerdict::kContinue || count == 0) return verdict_;
    const double frame_ms = 1000.0 * static_cast<double>(count) / static_cast<double>(rate_);
    const double level = rms(samples, count);
    last_level_ = level;
    elapsed_ms_ += frame_ms;
    if (elapsed_ms_ <= static_cast<double>(s_.calibrate_ms)) {   // the room
      if (!have_noise_ || level < noise_) noise_ = level;
      have_noise_ = true;
      return verdict_;
    }
    if (!calibrated_) {
      calibrated_ = true;
      if (noise_ > static_cast<double>(s_.min_level) * 5.0) { eager_ = true; speech_ = true; quiet_ms_ = 0.0; }   // somebody is already talking
    }
    const bool voice = level >= (speech_ ? end_threshold() : threshold());
    if (!speech_) {
      // voice adds up; quiet takes half as much away, so words with pauses between them add up and scattered noises (a door, a cough now and then) never do
      if (voice) voiced_ms_ += frame_ms;
      else voiced_ms_ = voiced_ms_ > frame_ms * 0.5 ? voiced_ms_ - frame_ms * 0.5 : 0.0;
      if (voiced_ms_ >= static_cast<double>(s_.min_speech_ms)) { speech_ = true; quiet_ms_ = 0.0; }
      else if (elapsed_ms_ >= static_cast<double>(s_.lead_ms)) verdict_ = SpeechVerdict::kNoSpeech;
      return verdict_;
    }
    quiet_ms_ = voice ? 0.0 : quiet_ms_ + frame_ms;
    if (quiet_ms_ >= static_cast<double>(s_.silence_ms)) verdict_ = SpeechVerdict::kSpeechEnded;
    return verdict_;
  }

  SpeechVerdict verdict() const { return verdict_; }
  bool speech_started() const { return speech_; }
  double elapsed_ms() const { return elapsed_ms_; }
  // The level of the room as measured, and the level a frame must reach to be a voice.
  double noise_level() const { return have_noise_ ? noise_ : 0.0; }
  double room_level() const { return have_noise_ && !eager_ ? noise_ : 0.0; }
  // The level a frame must reach to be a voice that starts a request.
  double threshold() const {
    const double wanted = room_level() * static_cast<double>(s_.level_percent) / 100.0;
    return wanted > static_cast<double>(s_.min_level) ? wanted : static_cast<double>(s_.min_level);
  }
  // Once the request has started, the soft end of a word still counts as voice: 70 % of that level, and never within a quarter of the room's own level.
  double end_threshold() const {
    const double soft = 0.7 * threshold();
    const double above_room = 1.25 * room_level();
    return soft > above_room ? soft : above_room;
  }
  double last_level() const { return last_level_; }

  static double rms(const std::int16_t* samples, std::size_t count) {
    if (count == 0) return 0.0;
    double sum = 0.0;
    for (std::size_t i = 0; i < count; ++i) sum += static_cast<double>(samples[i]) * static_cast<double>(samples[i]);
    return std::sqrt(sum / static_cast<double>(count));
  }

 private:
  SpeechEndSettings s_;
  unsigned rate_;
  SpeechVerdict verdict_ = SpeechVerdict::kContinue;
  double elapsed_ms_ = 0.0, noise_ = 0.0, voiced_ms_ = 0.0, quiet_ms_ = 0.0, last_level_ = 0.0;
  bool have_noise_ = false, speech_ = false, calibrated_ = false, eager_ = false;
};

}  // namespace armor::hmi
