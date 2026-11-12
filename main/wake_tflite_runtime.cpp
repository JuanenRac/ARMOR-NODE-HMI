// ARMOR-NODE-HMI - the TFLite Micro interpreter behind the Runtime interface of core/wake_tflite_runtime.hpp.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// Compiled ONLY when the build is configured with -DARMOR_WAKE_TFLITE=ON (an option of main/CMakeLists.txt, off by default) and the component espressif/esp-tflite-micro is part of the project: with the switch off this file is empty, so no
// image of the panel changes. The rules around the interpreter (what is a model, the sizes of the tensors, a number that is not a number) are tested on a computer in the core file with a
// stand-in for the interpreter; this file is only the thin part that talks to the library, and it has NOT run on a board with a model of the owner.
#if defined(ARMOR_WAKE_TFLITE) && ARMOR_WAKE_TFLITE

#include <cstdlib>
#include <cstring>
#include <memory>

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "core/wake_tflite_runtime.hpp"

namespace armor::wake_tflite {
namespace {

constexpr char kTag[] = "wake-tflite";

// One interpreter over one model that lives in memory the owner's loader provides (it must stay there for as long as the interpreter lives). The operators are the ones a small speech
// embedding and a small classifier use; a model that needs another one does not load (load() returns false and the panel does not listen).
class TflmRuntime : public Runtime {
 public:
  ~TflmRuntime() override { if (arena_ != nullptr) heap_caps_free(arena_); }

  bool load(const std::uint8_t* model, std::size_t model_bytes, std::size_t arena_bytes) override {
    (void)model_bytes;
    const tflite::Model* parsed = tflite::GetModel(model);
    if (parsed == nullptr || parsed->version() != TFLITE_SCHEMA_VERSION) { ESP_LOGW(kTag, "the model has another schema version"); return false; }
    arena_ = static_cast<std::uint8_t*>(heap_caps_malloc(arena_bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));   // the large tensors go to the external RAM
    if (arena_ == nullptr) { ESP_LOGW(kTag, "no memory for the arena of %u bytes", static_cast<unsigned>(arena_bytes)); return false; }
    if (resolver_.AddFullyConnected() != kTfLiteOk || resolver_.AddReshape() != kTfLiteOk || resolver_.AddConv2D() != kTfLiteOk || resolver_.AddDepthwiseConv2D() != kTfLiteOk ||
        resolver_.AddRelu() != kTfLiteOk || resolver_.AddLogistic() != kTfLiteOk || resolver_.AddSoftmax() != kTfLiteOk || resolver_.AddMean() != kTfLiteOk ||
        resolver_.AddQuantize() != kTfLiteOk || resolver_.AddDequantize() != kTfLiteOk || resolver_.AddMaxPool2D() != kTfLiteOk || resolver_.AddAdd() != kTfLiteOk) return false;
    interpreter_ = std::make_unique<tflite::MicroInterpreter>(parsed, resolver_, arena_, arena_bytes);
    if (interpreter_->AllocateTensors() != kTfLiteOk) { ESP_LOGW(kTag, "the model does not fit the arena"); interpreter_.reset(); return false; }
    return interpreter_->input(0) != nullptr && interpreter_->output(0) != nullptr && interpreter_->input(0)->type == kTfLiteFloat32 && interpreter_->output(0)->type == kTfLiteFloat32;
  }
  std::size_t input_floats() const override { return interpreter_ ? interpreter_->input(0)->bytes / sizeof(float) : 0; }
  std::size_t output_floats() const override { return interpreter_ ? interpreter_->output(0)->bytes / sizeof(float) : 0; }
  bool invoke(const float* in, float* out) override {
    if (!interpreter_) return false;
    std::memcpy(interpreter_->input(0)->data.f, in, interpreter_->input(0)->bytes);
    if (interpreter_->Invoke() != kTfLiteOk) return false;
    std::memcpy(out, interpreter_->output(0)->data.f, interpreter_->output(0)->bytes);
    return true;
  }

 private:
  tflite::MicroMutableOpResolver<12> resolver_;
  std::uint8_t* arena_ = nullptr;
  std::unique_ptr<tflite::MicroInterpreter> interpreter_;
};

}  // namespace

std::unique_ptr<Runtime> make_tflm_runtime() { return std::make_unique<TflmRuntime>(); }

}  // namespace armor::wake_tflite

#endif  // ARMOR_WAKE_TFLITE
