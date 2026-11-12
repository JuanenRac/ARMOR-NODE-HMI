// ARMOR-NODE-HMI - host tests of the model of the wake word as the board runs it (core/wake_tflite_runtime.hpp): a block of bytes that is or is not a model, an interpreter that is a stand-in,
// the sizes of the tensors, the failures of the interpreter and the whole detector built on two such models.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <vector>

#include "../core/wake_local.hpp"
#include "../core/wake_tflite.hpp"
#include "../core/wake_tflite_runtime.hpp"
#include "check.hpp"

using namespace armor::wake_tflite;

// A block that has the shape of a model: long enough, the identifier at byte 4.
static std::vector<std::uint8_t> model_block(std::size_t size = 4096) {
  std::vector<std::uint8_t> bytes(size, 0x11);
  if (size >= 8) { bytes[4] = 'T'; bytes[5] = 'F'; bytes[6] = 'L'; bytes[7] = '3'; }
  return bytes;
}

// The part of the interpreter: it loads when the arena is big enough and says the tensors it was made with; what it does with the numbers is a function.
struct Counters { unsigned loads = 0, invokes = 0; std::size_t last_arena = 0; };
using Fn = std::function<bool(const float*, float*)>;
class FakeRuntime : public Runtime {
 public:
  FakeRuntime(Counters* counters, std::size_t in, std::size_t out, std::size_t arena_needed, Fn fn) : counters_(counters), in_(in), out_(out), needed_(arena_needed), fn_(std::move(fn)) {}
  bool load(const std::uint8_t*, std::size_t, std::size_t arena) override { ++counters_->loads; counters_->last_arena = arena; return arena >= needed_; }
  std::size_t input_floats() const override { return in_; }
  std::size_t output_floats() const override { return out_; }
  bool invoke(const float* in, float* out) override { ++counters_->invokes; return fn_(in, out); }

 private:
  Counters* counters_;
  std::size_t in_, out_, needed_;
  Fn fn_;
};

static Fn half() { return [](const float*, float* o) { o[0] = 0.5f; return true; }; }
static std::unique_ptr<Runtime> runtime(Counters* c, std::size_t in, std::size_t out, std::size_t arena_needed = 1000, Fn fn = half()) {
  return std::make_unique<FakeRuntime>(c, in, out, arena_needed, std::move(fn));
}

static void test_what_is_a_model() {
  const auto block = model_block();
  CHECK(looks_like_a_model(block.data(), block.size()));
  CHECK(!looks_like_a_model(nullptr, 4096));
  CHECK(!looks_like_a_model(block.data(), kMinModelBytes - 1) && looks_like_a_model(block.data(), kMinModelBytes));
  CHECK(!looks_like_a_model(block.data(), kMaxModelBytes + 1));
  auto other = block;
  other[7] = '2';
  CHECK(!looks_like_a_model(other.data(), other.size()));   // another identifier
  other = block;
  other[4] = 0;
  CHECK(!looks_like_a_model(other.data(), other.size()));
}

static void test_the_model_is_ready_only_when_everything_fits() {
  const auto block = model_block();
  const auto tiny = model_block(10);
  Counters c;
  RuntimeModel good(runtime(&c, 8, 2), block.data(), block.size(), 4000, 8, 2);
  CHECK(good.ready() && good.input_size() == 8 && good.output_size() == 2 && c.loads == 1 && c.last_arena == 4000);
  Counters n1;
  RuntimeModel not_a_model(runtime(&n1, 8, 2), tiny.data(), tiny.size(), 4000, 8, 2);
  CHECK(!not_a_model.ready() && n1.loads == 0);                     // the interpreter is never asked to load a block that is not a model
  Counters n2;
  RuntimeModel no_arena(runtime(&n2, 8, 2), block.data(), block.size(), 0, 8, 2);
  CHECK(!no_arena.ready() && n2.loads == 0);
  Counters n3;
  RuntimeModel small_arena(runtime(&n3, 8, 2, 100000), block.data(), block.size(), 4000, 8, 2);
  CHECK(!small_arena.ready() && n3.loads == 1);                     // it did not fit the memory it was given
  Counters n4;
  RuntimeModel other_tensors(runtime(&n4, 9, 2), block.data(), block.size(), 4000, 8, 2);
  CHECK(!other_tensors.ready());                                    // not the model that was asked for
  Counters n5;
  RuntimeModel other_output(runtime(&n5, 8, 3), block.data(), block.size(), 4000, 8, 2);
  CHECK(!other_output.ready());
  RuntimeModel none(nullptr, block.data(), block.size(), 4000, 8, 2);
  CHECK(!none.ready());
  Counters n6;
  RuntimeModel zero(runtime(&n6, 0, 0), block.data(), block.size(), 4000, 0, 0);
  CHECK(!zero.ready());
}

static void test_a_run_goes_through_the_checks() {
  const auto block = model_block();
  Counters c;
  RuntimeModel model(runtime(&c, 3, 2, 1000, [](const float* in, float* out) { out[0] = in[0] + in[1] + in[2]; out[1] = 1.0f; return true; }), block.data(), block.size(), 4000, 3, 2);
  float out[2] = {0, 0};
  const float in[3] = {1.0f, 2.0f, 3.0f};
  CHECK(model.run(in, out) && out[0] == 6.0f && out[1] == 1.0f && c.invokes == 1);
  CHECK(!model.run(nullptr, out) && !model.run(in, nullptr) && c.invokes == 1);                    // nothing reaches the interpreter without buffers
  const float bad[3] = {1.0f, std::numeric_limits<float>::quiet_NaN(), 3.0f};
  CHECK(!model.run(bad, out) && model.refused() == 1 && c.invokes == 1);                           // a number that is not a number does not go in
  const float inf[3] = {1.0f, std::numeric_limits<float>::infinity(), 3.0f};
  CHECK(!model.run(inf, out) && model.refused() == 2);
  Counters n;
  RuntimeModel unready(runtime(&n, 3, 2, 100000), block.data(), block.size(), 4000, 3, 2);
  CHECK(!unready.run(in, out) && n.invokes == 0);                                                   // a model that is not ready is never run
}

static void test_a_failing_or_wild_interpreter() {
  const auto block = model_block();
  Counters c;
  bool fail = true;
  RuntimeModel model(runtime(&c, 2, 2, 1000, [&fail](const float*, float* out) { if (fail) return false; out[0] = std::numeric_limits<float>::quiet_NaN(); out[1] = 0.25f; return true; }),
                     block.data(), block.size(), 4000, 2, 2);
  const float in[2] = {0.1f, 0.2f};
  float out[2] = {7.0f, 7.0f};
  CHECK(!model.run(in, out) && model.failures() == 1);                                              // the interpreter said no
  fail = false;
  CHECK(model.run(in, out) && out[0] == 0.0f && out[1] == 0.25f && model.cleaned() == 1);           // a number that is not a number never leaves it
}

// The whole detector on two models made this way: nothing is heard until the models are ready, a missing model gives a score of zero, and the score is the classifier's once both have run.
static void test_the_detector_on_runtime_models() {
  const Settings s;
  const auto block = model_block();
  Counters ce, cc;
  RuntimeModel embedding(runtime(&ce, s.mel_frames * s.mels, s.embedding_size, 1000, [s](const float*, float* out) { for (std::size_t i = 0; i < s.embedding_size; ++i) out[i] = 3.0f; return true; }),
                         block.data(), block.size(), 8000, s.mel_frames * s.mels, s.embedding_size);
  RuntimeModel classifier(runtime(&cc, s.embeddings * s.embedding_size, 1, 1000, [](const float*, float* out) { out[0] = 0.8f; return true; }), block.data(), block.size(), 8000,
                          s.embeddings * s.embedding_size, 1);
  Detector detector(&embedding, &classifier, s);
  CHECK(detector.ready());
  std::vector<std::int16_t> sound(armor::wake_local::kFrameSamples, 0);
  for (std::size_t i = 0; i < sound.size(); ++i) sound[i] = static_cast<std::int16_t>(8000.0 * std::sin(2.0 * 3.14159265358979 * 440.0 * static_cast<double>(i) / 16000.0));
  float last = 0.0f;
  for (int frame = 0; frame < 400; ++frame) last = detector.score(sound.data(), sound.size());
  CHECK(last > 0.79f && last < 0.81f && ce.invokes > 0 && cc.invokes > 0);
  Counters missing;
  RuntimeModel absent(runtime(&missing, s.embeddings * s.embedding_size, 1, 100000), block.data(), block.size(), 8000, s.embeddings * s.embedding_size, 1);
  Detector without(&embedding, &absent, s);
  CHECK(!without.ready());
  float score = 1.0f;
  for (int frame = 0; frame < 50; ++frame) score = without.score(sound.data(), sound.size());
  CHECK(score == 0.0f);                                                                              // a wake is never invented
}

int main() {
  test_what_is_a_model();
  test_the_model_is_ready_only_when_everything_fits();
  test_a_run_goes_through_the_checks();
  test_a_failing_or_wild_interpreter();
  test_the_detector_on_runtime_models();
  FINISH("test_wake_runtime");
}
