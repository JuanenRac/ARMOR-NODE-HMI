// ARMOR-NODE-HMI - the detector of the wake word that a TFLite Micro model plugs into: the front end that turns the microphone into mel frames, the chain of two models (embedding, then
// classifier) and the rule that makes a score of it. Pure: tested on a computer with models that are functions. Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// core/wake_local.hpp defines the detector the panel listens with (a score from 0 to 1 for each frame of 80 ms). This is the detector that fills it: the sound goes through a mel front end
// written here, and the mel frames through two small models that the runtime of the board runs (the embedding and the classifier of an openWakeWord style chain). The runtime is behind the
// `Model` interface below, so this file does not include it: the firmware offers a detector only when the TFLite Micro runtime is linked AND the owner's models are on the board (Model::ready()),
// and until then voice.wake_local does nothing and the service is used. What is NOT here, and has not run on a board: the Model that wraps the TFLite Micro interpreter, the models
// themselves (they are trained by the owner) and the proof that the front end below gives the numbers the models were trained with. The sizes and the front end are settings of the
// detector, never fixed in the program: they must be the ones of the training, and they are checked when the detector is made.
#pragma once
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <utility>
#include <vector>

#include "wake_local.hpp"

namespace armor::wake_tflite {

// What the front end and the chain are built with; the defaults are the ones of the openWakeWord chain (16 kHz, 32 mel bands, 10 ms hop, 76 mel frames to one embedding of 96 numbers,
// 16 embeddings to one score). A model trained with other values needs other settings here.
struct Settings {
  std::size_t window = 400;           // samples of one analysis window (25 ms)
  std::size_t hop = 160;              // samples between two windows (10 ms)
  std::size_t fft = 512;              // a power of two, at least the window
  std::size_t mels = 32;              // mel bands
  float low_hz = 60.0f;
  float high_hz = 3800.0f;
  float scale_div = 10.0f;            // the features are log10(mel power) / scale_div + scale_add, the range the embedding model was trained in
  float scale_add = 2.0f;
  std::size_t mel_frames = 76;        // the mel frames one embedding looks at
  std::size_t embedding_size = 96;    // the numbers one embedding has
  std::size_t embeddings = 16;        // the embeddings one score looks at
};

// True when the settings can run: a power-of-two FFT that holds the window, a hop that moves, bands inside the spectrum, and sizes that are not empty or absurd for a board.
inline bool valid(const Settings& s) {
  const bool power_of_two = s.fft >= 64 && s.fft <= 4096 && (s.fft & (s.fft - 1)) == 0;
  return power_of_two && s.window >= 64 && s.window <= s.fft && s.hop >= 16 && s.hop <= s.window && s.mels >= 8 && s.mels <= 80 && std::isfinite(s.low_hz) && std::isfinite(s.high_hz) &&
         s.low_hz >= 0.0f && s.low_hz < s.high_hz && s.high_hz <= static_cast<float>(wake_local::kRate) / 2.0f && std::isfinite(s.scale_div) && s.scale_div > 0.0f &&
         std::isfinite(s.scale_add) && s.mel_frames >= 4 && s.mel_frames <= 200 && s.embedding_size >= 8 && s.embedding_size <= 512 && s.embeddings >= 2 && s.embeddings <= 64;
}

// A model the runtime of the board runs: tensors of floats in and out. The firmware wraps its interpreter in this; the tests use functions.
class Model {
 public:
  virtual ~Model() = default;
  virtual bool ready() const = 0;                       // loaded and able to run
  virtual std::size_t input_size() const = 0;           // floats it takes
  virtual std::size_t output_size() const = 0;          // floats it gives
  virtual bool run(const float* in, float* out) = 0;    // false when it could not run (the score of that frame is then 0)
};

// A model made of a function, for the computer (and for a board whose model is missing: ready false).
class FunctionModel : public Model {
 public:
  using Fn = std::function<bool(const float*, float*)>;
  FunctionModel(std::size_t input, std::size_t output, Fn fn, bool loaded = true) : in_(input), out_(output), fn_(std::move(fn)), loaded_(loaded) {}
  bool ready() const override { return loaded_ && static_cast<bool>(fn_); }
  std::size_t input_size() const override { return in_; }
  std::size_t output_size() const override { return out_; }
  bool run(const float* in, float* out) override { ++runs_; return fn_ ? fn_(in, out) : false; }
  unsigned runs() const { return runs_; }

 private:
  std::size_t in_, out_;
  Fn fn_;
  bool loaded_;
  unsigned runs_ = 0;
};

// Sound in, mel frames out. Keeps only what the next window needs (less than one window of samples); no sound is stored beyond that.
class MelFrontend {
 public:
  explicit MelFrontend(const Settings& s = Settings()) : s_(s), ok_(valid(s)) {
    if (!ok_) return;
    window_.resize(s_.window);
    for (std::size_t i = 0; i < s_.window; ++i) window_[i] = 0.5f - 0.5f * std::cos(2.0f * kPi * static_cast<float>(i) / static_cast<float>(s_.window));   // Hann
    cos_.resize(s_.fft / 2);
    sin_.resize(s_.fft / 2);
    for (std::size_t i = 0; i < s_.fft / 2; ++i) {
      const float angle = -2.0f * kPi * static_cast<float>(i) / static_cast<float>(s_.fft);
      cos_[i] = std::cos(angle);
      sin_[i] = std::sin(angle);
    }
    build_bands();
    re_.resize(s_.fft);
    im_.resize(s_.fft);
  }
  bool ok() const { return ok_; }
  std::size_t mels() const { return s_.mels; }
  void reset() { pending_.clear(); }
  // Offers samples and appends the mel frames they complete to `out` (mels floats each). Returns how many frames were added.
  std::size_t feed(const std::int16_t* samples, std::size_t n, std::vector<float>& out) {
    if (!ok_ || samples == nullptr) return 0;
    pending_.insert(pending_.end(), samples, samples + n);
    std::size_t frames = 0, at = 0;
    while (pending_.size() - at >= s_.window) {
      one(pending_.data() + at, out);
      at += s_.hop;
      ++frames;
    }
    pending_.erase(pending_.begin(), pending_.begin() + static_cast<std::ptrdiff_t>(at));
    return frames;
  }

 private:
  static constexpr float kPi = 3.14159265358979323846f;
  static float hz_to_mel(float hz) { return 2595.0f * std::log10(1.0f + hz / 700.0f); }
  static float mel_to_hz(float mel) { return 700.0f * (std::pow(10.0f, mel / 2595.0f) - 1.0f); }

  void build_bands() {
    // triangular bands on the mel scale between low_hz and high_hz: band b rises from point b to point b+1 and falls to point b+2
    const std::size_t bins = s_.fft / 2 + 1;
    const float rate = static_cast<float>(wake_local::kRate);
    const float low = hz_to_mel(s_.low_hz), high = hz_to_mel(s_.high_hz);
    std::vector<float> edge(s_.mels + 2);
    for (std::size_t i = 0; i < edge.size(); ++i) edge[i] = mel_to_hz(low + (high - low) * static_cast<float>(i) / static_cast<float>(s_.mels + 1));
    weights_.assign(s_.mels * bins, 0.0f);
    for (std::size_t b = 0; b < s_.mels; ++b)
      for (std::size_t k = 0; k < bins; ++k) {
        const float hz = rate * static_cast<float>(k) / static_cast<float>(s_.fft);
        float w = 0.0f;
        if (hz > edge[b] && hz <= edge[b + 1]) w = (hz - edge[b]) / (edge[b + 1] - edge[b]);
        else if (hz > edge[b + 1] && hz < edge[b + 2]) w = (edge[b + 2] - hz) / (edge[b + 2] - edge[b + 1]);
        weights_[b * bins + k] = w;
      }
  }

  void fft_in_place() {
    const std::size_t n = s_.fft;
    for (std::size_t i = 1, j = 0; i < n; ++i) {   // bit reversal
      std::size_t bit = n >> 1;
      for (; j & bit; bit >>= 1) j ^= bit;
      j ^= bit;
      if (i < j) { std::swap(re_[i], re_[j]); std::swap(im_[i], im_[j]); }
    }
    for (std::size_t len = 2; len <= n; len <<= 1) {
      const std::size_t step = n / len;
      for (std::size_t i = 0; i < n; i += len)
        for (std::size_t k = 0; k < len / 2; ++k) {
          const float wr = cos_[k * step], wi = sin_[k * step];
          const std::size_t a = i + k, b = i + k + len / 2;
          const float tr = re_[b] * wr - im_[b] * wi, ti = re_[b] * wi + im_[b] * wr;
          re_[b] = re_[a] - tr; im_[b] = im_[a] - ti;
          re_[a] += tr; im_[a] += ti;
        }
    }
  }

  void one(const std::int16_t* samples, std::vector<float>& out) {
    for (std::size_t i = 0; i < s_.fft; ++i) {
      re_[i] = i < s_.window ? static_cast<float>(samples[i]) * window_[i] : 0.0f;
      im_[i] = 0.0f;
    }
    fft_in_place();
    const std::size_t bins = s_.fft / 2 + 1;
    for (std::size_t b = 0; b < s_.mels; ++b) {
      float energy = 0.0f;
      for (std::size_t k = 0; k < bins; ++k) {
        const float w = weights_[b * bins + k];
        if (w > 0.0f) energy += w * (re_[k] * re_[k] + im_[k] * im_[k]);
      }
      out.push_back(std::log10(energy > 1e-10f ? energy : 1e-10f) / s_.scale_div + s_.scale_add);
    }
  }

  Settings s_;
  bool ok_;
  std::vector<float> window_, cos_, sin_, weights_, re_, im_;
  std::vector<std::int16_t> pending_;
};

// The detector: the front end, the embedding model and the classifier model. It scores one frame of 80 ms at a time, as wake_local::Listener gives them: the mel frames of the frame
// are added to the last `mel_frames`; once there are that many the embedding model runs on them, and once there are `embeddings` embeddings the classifier runs on those and gives the
// score. Before that, and whenever a model cannot run, the score is 0 (a wake is never invented). The numbers a model gives are held to 0..1 and a number that is not a number is 0.
class Detector : public wake_local::Detector {
 public:
  Detector(Model* embedding, Model* classifier, const Settings& s = Settings()) : s_(s), front_(s), embedding_(embedding), classifier_(classifier) {
    ok_ = front_.ok() && embedding_ != nullptr && classifier_ != nullptr && embedding_->input_size() == s_.mel_frames * s_.mels && embedding_->output_size() == s_.embedding_size &&
          classifier_->input_size() == s_.embeddings * s_.embedding_size && classifier_->output_size() == 1;
  }
  // Ready only when the sizes of both models are the ones of the settings and both are loaded.
  bool ready() const override { return ok_ && embedding_->ready() && classifier_->ready(); }
  float score(const std::int16_t* frame, std::size_t samples) override {
    if (!ready() || frame == nullptr) return 0.0f;
    front_.feed(frame, samples, mel_);
    const std::size_t keep = s_.mel_frames * s_.mels;
    if (mel_.size() > keep) mel_.erase(mel_.begin(), mel_.end() - static_cast<std::ptrdiff_t>(keep));
    if (mel_.size() < keep) return 0.0f;
    std::vector<float> fresh(s_.embedding_size);
    if (!embedding_->run(mel_.data(), fresh.data())) return 0.0f;
    embeds_.insert(embeds_.end(), fresh.begin(), fresh.end());
    const std::size_t want = s_.embeddings * s_.embedding_size;
    if (embeds_.size() > want) embeds_.erase(embeds_.begin(), embeds_.end() - static_cast<std::ptrdiff_t>(want));
    if (embeds_.size() < want) return 0.0f;
    float result = 0.0f;
    if (!classifier_->run(embeds_.data(), &result) || !(result > 0.0f)) return 0.0f;   // also catches NaN
    return result > 1.0f ? 1.0f : result;
  }
  // Forgets the sound heard so far (after a wake, or when the microphone was off).
  void reset() { front_.reset(); mel_.clear(); embeds_.clear(); }
  std::size_t mel_frames_held() const { return mel_.size() / s_.mels; }
  std::size_t embeddings_held() const { return embeds_.size() / s_.embedding_size; }

 private:
  Settings s_;
  MelFrontend front_;
  Model* embedding_;
  Model* classifier_;
  bool ok_ = false;
  std::vector<float> mel_, embeds_;
};

}  // namespace armor::wake_tflite
