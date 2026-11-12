// ARMOR-NODE-HMI - the wake word heard on the panel itself: the interface of a detector, the rule that turns its scores into a wake, and a detector for the computer that plays a script.
// Pure: tested on a computer. Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// The default way (core/wake_stream.hpp) sends short pieces of sound to the voice service, which runs openWakeWord. This is the other way, for a panel that must not send sound at all:
// a small model on the board listens to the microphone in frames of 80 ms and only a wake leaves it. The setting that asks for it (voice.wake_local) is OFF by default, and it does nothing
// unless a detector says it is ready: the firmware offers none until a model trained by the owner (ARMOR-VOICE-AI docs/WAKE_TRAINING.md) is on the board and the TFLite Micro runtime
// is linked, so today a panel with the setting on keeps using the service. The real detector (TFLite Micro on the ESP32-S3) is NOT written here and has not run on a board; this file is
// what it plugs into, and the simulated detector below is what the tests run against.
#pragma once
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace armor::wake_local {

constexpr std::size_t kRate = 16000;
constexpr std::size_t kFrameSamples = kRate * 80 / 1000;   // 80 ms, the frame openWakeWord also works in

// What a model on the board has to give: a score from 0 to 1 for each frame of 80 ms, and whether it is loaded and able to run.
class Detector {
 public:
  virtual ~Detector() = default;
  virtual bool ready() const = 0;
  virtual float score(const std::int16_t* frame, std::size_t samples) = 0;
};

// Turns scores into a wake: the score has to stay at or above the threshold for `frames` frames in a row (one noisy frame wakes nothing), and after a wake the panel is left alone
// for `refractory_ms`. The values are the ones the owner tunes in the settings, never fixed in the program.
class Trigger {
 public:
  struct Settings {
    float threshold = 0.5f;               // 0.05 to 0.99
    unsigned frames = 2;                  // 1 to 10
    std::uint32_t refractory_ms = 3000;   // 500 to 30000
  };
  Trigger() = default;
  explicit Trigger(Settings s) : s_(clamped(s)) {}
  static Settings clamped(Settings s) {
    if (!(s.threshold >= 0.05f)) s.threshold = 0.05f;   // also catches NaN
    if (s.threshold > 0.99f) s.threshold = 0.99f;
    if (s.frames < 1) s.frames = 1;
    if (s.frames > 10) s.frames = 10;
    if (s.refractory_ms < 500) s.refractory_ms = 500;
    if (s.refractory_ms > 30000) s.refractory_ms = 30000;
    return s;
  }
  const Settings& settings() const { return s_; }
  // True on the frame that completes a wake.
  bool update(float score, std::uint32_t now) {
    if (quiet_ && static_cast<std::int32_t>(now - quiet_until_) < 0) { run_ = 0; return false; }
    quiet_ = false;
    if (score >= s_.threshold) ++run_; else run_ = 0;
    if (run_ >= s_.frames) { run_ = 0; quiet_ = true; quiet_until_ = now + s_.refractory_ms; return true; }
    return false;
  }
  void reset() { run_ = 0; quiet_ = false; }

 private:
  Settings s_{};
  unsigned run_ = 0;
  bool quiet_ = false;
  std::uint32_t quiet_until_ = 0;
};

// How the panel listens for the word. A local detector is used only when the setting is on AND the detector is ready; otherwise the service is used when voice.wake_word is on,
// and otherwise the panel waits for a touch.
enum class Source { kNone, kService, kLocal };
inline Source choose_source(bool wake_local_setting, const Detector* detector, bool wake_word_setting) {
  if (wake_local_setting && detector != nullptr && detector->ready()) return Source::kLocal;
  return wake_word_setting ? Source::kService : Source::kNone;
}

// Cuts the microphone's samples into frames of 80 ms, scores each and reports a wake. Nothing is kept except the frame being filled: no sound is stored or sent.
class Listener {
 public:
  Listener(Detector* detector, Trigger trigger) : detector_(detector), trigger_(trigger) {}
  bool available() const { return detector_ != nullptr && detector_->ready(); }
  // Offers samples; true when one of the frames completed a wake. The samples after a wake in the same call are dropped (the panel starts a turn now).
  bool feed(const std::int16_t* samples, std::size_t n, std::uint32_t now) {
    if (!available() || samples == nullptr) return false;
    for (std::size_t i = 0; i < n; ++i) {
      frame_[fill_++] = samples[i];
      if (fill_ < kFrameSamples) continue;
      fill_ = 0;
      const float score = detector_->score(frame_, kFrameSamples);
      if (trigger_.update(score, now)) return true;
    }
    return false;
  }
  void reset() { fill_ = 0; trigger_.reset(); }

 private:
  Detector* detector_;
  Trigger trigger_;
  std::int16_t frame_[kFrameSamples] = {};
  std::size_t fill_ = 0;
};

// The detector of the computer: no model, a script. Either it plays the scores it was given one frame at a time (then repeats the last), or, with no script, it scores a frame by the
// share of its samples at or above `loud_level` (a loud tone stands for the word). `loaded` false plays a board whose model is missing.
class SimulatedDetector : public Detector {
 public:
  explicit SimulatedDetector(bool loaded = true, std::int16_t loud_level = 8000) : loaded_(loaded), loud_level_(loud_level) {}
  void script(std::vector<float> scores) { script_ = std::move(scores); next_ = 0; }
  bool ready() const override { return loaded_; }
  float score(const std::int16_t* frame, std::size_t samples) override {
    ++frames_;
    if (!script_.empty()) { const float value = script_[next_ < script_.size() ? next_ : script_.size() - 1]; ++next_; return value; }
    if (samples == 0) return 0.0f;
    std::size_t loud = 0;
    for (std::size_t i = 0; i < samples; ++i) { const int v = frame[i] < 0 ? -static_cast<int>(frame[i]) : frame[i]; if (v >= loud_level_) ++loud; }
    return static_cast<float>(loud) / static_cast<float>(samples);
  }
  unsigned frames_scored() const { return frames_; }

 private:
  bool loaded_;
  std::int16_t loud_level_;
  std::vector<float> script_;
  std::size_t next_ = 0;
  unsigned frames_ = 0;
};

}  // namespace armor::wake_local
