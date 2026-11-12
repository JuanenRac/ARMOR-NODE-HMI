// ARMOR-NODE-HMI - the model of the wake word as the board runs it: a flat model (TFLite format) in a block of bytes, an interpreter behind an interface, and the checks around them.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// core/wake_tflite.hpp defines `Model` (floats in, floats out). The firmware fills it with `RuntimeModel`, which owns the rules that do not depend on the interpreter and are tested on a computer:
// a block that is not a model is refused (a TFLite file carries the identifier "TFL3" at byte 4), a model that does not fit the memory it is given (the arena) is not ready, a model whose tensors
// are not the ones the detector expects is not ready, a run with the wrong sizes does not reach the interpreter, and a number that is not a number never leaves it. The interpreter itself (TFLite Micro,
// main/wake_tflite_runtime.cpp) is only compiled when the build asks for it (the build option ARMOR_WAKE_TFLITE, off by default); on the computer a stand-in plays its part.
#pragma once
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>

#include "wake_tflite.hpp"

namespace armor::wake_tflite {

constexpr std::size_t kMinModelBytes = 64;                   // smaller than any real model: a block this small is not one
constexpr std::size_t kMaxModelBytes = 2 * 1024 * 1024;      // larger than the board is meant to hold for a wake word

// What the interpreter does for the model: load the block into an arena of the given size and say what the tensors are; run it.
class Runtime {
 public:
  virtual ~Runtime() = default;
  virtual bool load(const std::uint8_t* model, std::size_t model_bytes, std::size_t arena_bytes) = 0;   // false: the model could not be set up (too big for the arena, an operator the build lacks)
  virtual std::size_t input_floats() const = 0;
  virtual std::size_t output_floats() const = 0;
  virtual bool invoke(const float* in, float* out) = 0;                                               // false: the interpreter reported an error
};

// True when the block has the shape of a TFLite model: long enough, short enough and the file identifier at byte 4.
inline bool looks_like_a_model(const std::uint8_t* bytes, std::size_t size) {
  if (bytes == nullptr || size < kMinModelBytes || size > kMaxModelBytes) return false;
  return bytes[4] == 'T' && bytes[5] == 'F' && bytes[6] == 'L' && bytes[7] == '3';
}

// The Model the detector uses, made of a block of bytes and an interpreter. `want_in` and `want_out` are the sizes of the tensors the detector needs (the embedding model takes the mel frames and
// gives an embedding; the classifier takes the embeddings and gives one score).
class RuntimeModel : public Model {
 public:
  RuntimeModel(std::unique_ptr<Runtime> runtime, const std::uint8_t* model, std::size_t model_bytes, std::size_t arena_bytes, std::size_t want_in, std::size_t want_out)
      : runtime_(std::move(runtime)), in_(want_in), out_(want_out) {
    if (!runtime_ || !looks_like_a_model(model, model_bytes) || arena_bytes == 0) return;
    if (!runtime_->load(model, model_bytes, arena_bytes)) return;
    ready_ = runtime_->input_floats() == want_in && runtime_->output_floats() == want_out && want_in > 0 && want_out > 0;   // other tensors: this is not the model that was asked for
  }
  bool ready() const override { return ready_; }
  std::size_t input_size() const override { return in_; }
  std::size_t output_size() const override { return out_; }
  bool run(const float* in, float* out) override {
    if (!ready_ || in == nullptr || out == nullptr) return false;
    for (std::size_t i = 0; i < in_; ++i) if (!std::isfinite(in[i])) { ++refused_; return false; }   // a number that is not a number never reaches the interpreter
    if (!runtime_->invoke(in, out)) { ++failures_; return false; }
    for (std::size_t i = 0; i < out_; ++i) if (!std::isfinite(out[i])) { out[i] = 0.0f; ++cleaned_; }     // and never leaves it
    return true;
  }
  unsigned failures() const { return failures_; }
  unsigned refused() const { return refused_; }
  unsigned cleaned() const { return cleaned_; }

 private:
  std::unique_ptr<Runtime> runtime_;
  std::size_t in_, out_;
  bool ready_ = false;
  unsigned failures_ = 0, refused_ = 0, cleaned_ = 0;
};

}  // namespace armor::wake_tflite
