// ARMOR-NODE-HMI - host tests of the detector a TFLite Micro model plugs into: the mel front end, the chain of two models and the score, against models that are functions.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

#include "../core/wake_local.hpp"
#include "../core/wake_tflite.hpp"
#include "check.hpp"

using namespace armor::wake_tflite;
using armor::wake_local::kFrameSamples;

static std::size_t g_tone_at = 0;
static std::vector<std::int16_t> tone(std::size_t n, double hz, double amplitude = 12000.0) {
  std::vector<std::int16_t> out(n);
  for (std::size_t i = 0; i < n; ++i, ++g_tone_at)
    out[i] = static_cast<std::int16_t>(amplitude * std::sin(2.0 * 3.14159265358979323846 * hz * static_cast<double>(g_tone_at) / 16000.0));
  return out;
}
static std::vector<std::int16_t> silence(std::size_t n) { return std::vector<std::int16_t>(n, 0); }

static std::size_t argmax(const float* row, std::size_t n) {
  std::size_t best = 0;
  for (std::size_t i = 1; i < n; ++i) if (row[i] > row[best]) best = i;
  return best;
}

// A pair of models for the default settings: the embedding gives the strongest mel feature of its window, the classifier says 0.9 when the last embedding is strong and 0.1 when not.
struct Chain {
  Settings s;
  FunctionModel embedding;
  FunctionModel classifier;
  explicit Chain(const Settings& settings = Settings())
      : s(settings),
        embedding(settings.mel_frames * settings.mels, settings.embedding_size, [settings](const float* in, float* out) {
          float best = in[0];
          for (std::size_t i = 1; i < settings.mel_frames * settings.mels; ++i) if (in[i] > best) best = in[i];
          for (std::size_t i = 0; i < settings.embedding_size; ++i) out[i] = i == 0 ? best : 0.0f;
          return true;
        }),
        classifier(settings.embeddings * settings.embedding_size, 1, [settings](const float* in, float* out) {
          out[0] = in[(settings.embeddings - 1) * settings.embedding_size] > 2.0f ? 0.9f : 0.1f;
          return true;
        }) {}
};

static void test_the_settings_are_checked_before_anything_runs() {
  CHECK(valid(Settings()));
  Settings s;
  s.fft = 500; CHECK(!valid(s));                        // not a power of two
  s = Settings(); s.window = 1024; CHECK(!valid(s));    // the window does not fit in the transform
  s = Settings(); s.hop = 0; CHECK(!valid(s));
  s = Settings(); s.hop = 500; CHECK(!valid(s));        // a hop longer than the window skips sound
  s = Settings(); s.mels = 2; CHECK(!valid(s));
  s = Settings(); s.low_hz = 4000.0f; CHECK(!valid(s));
  s = Settings(); s.high_hz = 9000.0f; CHECK(!valid(s));   // over half the rate
  s = Settings(); s.low_hz = std::numeric_limits<float>::quiet_NaN(); CHECK(!valid(s));
  s = Settings(); s.scale_div = 0.0f; CHECK(!valid(s));
  s = Settings(); s.mel_frames = 1; CHECK(!valid(s));
  s = Settings(); s.embeddings = 1000; CHECK(!valid(s));
  MelFrontend bad(s);
  std::vector<float> out;
  CHECK(!bad.ok() && bad.feed(silence(2000).data(), 2000, out) == 0 && out.empty());
}

static void test_the_front_end_gives_one_frame_every_hop_and_keeps_less_than_a_window() {
  MelFrontend front;
  std::vector<float> out;
  const std::vector<std::int16_t> quiet = silence(kFrameSamples);
  CHECK(front.feed(quiet.data(), quiet.size(), out) == 6);    // 1280 samples complete six windows of 400 at a hop of 160
  CHECK(out.size() == 6 * front.mels());
  CHECK(front.feed(quiet.data(), quiet.size(), out) == 8);    // then eight for each 80 ms
  CHECK(front.feed(quiet.data(), quiet.size(), out) == 8);
  CHECK(front.feed(nullptr, 100, out) == 0);
  front.reset();
  CHECK(front.feed(quiet.data(), 399, out) == 0);             // one sample short of a window
  CHECK(front.feed(quiet.data(), 1, out) == 1);
}

static void test_silence_is_the_floor_and_a_tone_lights_the_band_of_its_pitch() {
  MelFrontend front;
  std::vector<float> out;
  const std::vector<std::int16_t> quiet = silence(kFrameSamples);
  front.feed(quiet.data(), quiet.size(), out);
  for (float v : out) CHECK(std::fabs(v - 1.0f) < 1e-4f);     // log10(1e-10) / 10 + 2
  const std::size_t mels = front.mels();
  auto peak = [&](double hz) {
    MelFrontend f;
    std::vector<float> frames;
    g_tone_at = 0;
    const std::vector<std::int16_t> sound = tone(kFrameSamples * 2, hz);
    f.feed(sound.data(), sound.size(), frames);
    const float* row = frames.data() + (frames.size() / mels - 1) * mels;
    return std::make_pair(argmax(row, mels), row[argmax(row, mels)]);
  };
  const auto low = peak(500.0), middle = peak(1000.0), high = peak(2500.0);
  CHECK(low.first < middle.first && middle.first < high.first);   // a higher pitch lights a higher band
  CHECK(middle.first >= 13 && middle.first <= 15);                // 1000 Hz sits near the middle of the mel scale between 60 and 3800 Hz
  CHECK(middle.second > 2.5f);                                    // well over the floor of 1.0
}

static void test_the_detector_says_nothing_until_both_windows_are_full() {
  Chain chain;
  Detector detector(&chain.embedding, &chain.classifier);
  CHECK(detector.ready());
  const std::vector<std::int16_t> quiet = silence(kFrameSamples);
  for (int call = 1; call <= 9; ++call) CHECK(detector.score(quiet.data(), quiet.size()) == 0.0f);
  CHECK(chain.embedding.runs() == 0);                            // 6 + 8 * 8 = 70 mel frames, not yet the 76 an embedding needs
  CHECK(detector.score(quiet.data(), quiet.size()) == 0.0f);     // the tenth call fills the window ...
  CHECK(chain.embedding.runs() == 1 && detector.mel_frames_held() == 76);
  for (int call = 11; call <= 24; ++call) CHECK(detector.score(quiet.data(), quiet.size()) == 0.0f);
  CHECK(chain.classifier.runs() == 0 && detector.embeddings_held() == 15);
  const float first = detector.score(quiet.data(), quiet.size());   // ... and the 25th holds the 16 embeddings the classifier looks at
  CHECK(std::fabs(first - 0.1f) < 1e-6f);
  CHECK(chain.classifier.runs() == 1 && detector.embeddings_held() == 16);
}

static void test_a_tone_after_silence_is_heard_at_once_and_reset_forgets_it() {
  Chain chain;
  Detector detector(&chain.embedding, &chain.classifier);
  const std::vector<std::int16_t> quiet = silence(kFrameSamples);
  for (int call = 1; call <= 30; ++call) detector.score(quiet.data(), quiet.size());
  CHECK(detector.score(quiet.data(), quiet.size()) < 0.5f);
  g_tone_at = 0;
  const std::vector<std::int16_t> sound = tone(kFrameSamples, 1000.0);
  CHECK(detector.score(sound.data(), sound.size()) > 0.8f);
  detector.reset();
  CHECK(detector.mel_frames_held() == 0 && detector.embeddings_held() == 0);
  CHECK(detector.score(sound.data(), sound.size()) == 0.0f);     // everything has to fill again
}

static void test_a_detector_with_missing_or_wrong_models_is_not_ready_and_never_scores() {
  Chain chain;
  Detector none(nullptr, nullptr);
  CHECK(!none.ready());
  FunctionModel missing(76 * 32, 96, [](const float*, float*) { return true; }, false);
  Detector unloaded(&missing, &chain.classifier);
  CHECK(!unloaded.ready());
  FunctionModel narrow(10, 96, [](const float*, float*) { return true; });
  Detector wrong_input(&narrow, &chain.classifier);
  CHECK(!wrong_input.ready());
  FunctionModel two_outputs(16 * 96, 2, [](const float*, float*) { return true; });
  Detector wrong_output(&chain.embedding, &two_outputs);
  CHECK(!wrong_output.ready());
  Settings broken;
  broken.fft = 500;
  Detector bad_settings(&chain.embedding, &chain.classifier, broken);
  CHECK(!bad_settings.ready());
  const std::vector<std::int16_t> sound = tone(kFrameSamples, 1000.0);
  for (int call = 0; call < 40; ++call) CHECK(unloaded.score(sound.data(), sound.size()) == 0.0f && wrong_input.score(sound.data(), sound.size()) == 0.0f);
  CHECK(chain.embedding.runs() == 0 && missing.runs() == 0);      // nothing ran
}

static void test_a_model_that_fails_or_gives_nonsense_gives_a_score_of_zero_or_a_held_one() {
  Chain chain;
  bool fail = false;
  FunctionModel flaky(76 * 32, 96, [&](const float* in, float* out) { if (fail) return false; return chain.embedding.run(in, out); });
  float answer = 0.7f;
  FunctionModel classifier(16 * 96, 1, [&](const float*, float* out) { out[0] = answer; return true; });
  Detector detector(&flaky, &classifier);
  CHECK(detector.ready());
  const std::vector<std::int16_t> quiet = silence(kFrameSamples);
  float last = 0.0f;
  for (int call = 1; call <= 25; ++call) last = detector.score(quiet.data(), quiet.size());
  CHECK(std::fabs(last - 0.7f) < 1e-6f);
  answer = 5.0f;  CHECK(detector.score(quiet.data(), quiet.size()) == 1.0f);                                    // held to 1
  answer = -3.0f; CHECK(detector.score(quiet.data(), quiet.size()) == 0.0f);                                    // and to 0
  answer = std::numeric_limits<float>::quiet_NaN(); CHECK(detector.score(quiet.data(), quiet.size()) == 0.0f);   // not a number is no wake
  answer = 0.7f;
  fail = true;
  CHECK(detector.score(quiet.data(), quiet.size()) == 0.0f);                                                    // a model that could not run
  CHECK(detector.score(nullptr, 100) == 0.0f);
}

static void test_the_listener_wakes_on_the_second_frame_of_a_tone_and_not_on_silence() {
  Chain chain;
  Detector detector(&chain.embedding, &chain.classifier);
  armor::wake_local::Listener listener(&detector, armor::wake_local::Trigger(armor::wake_local::Trigger::Settings{0.5f, 2, 1000}));
  CHECK(listener.available());
  const std::vector<std::int16_t> quiet = silence(kFrameSamples * 30);
  CHECK(!listener.feed(quiet.data(), quiet.size(), 0));
  g_tone_at = 0;
  const std::vector<std::int16_t> sound = tone(kFrameSamples, 1000.0);
  CHECK(!listener.feed(sound.data(), sound.size(), 2400));       // one strong frame is not a wake
  CHECK(listener.feed(sound.data(), sound.size(), 2480));        // the second in a row is
  CHECK(armor::wake_local::choose_source(true, &detector, false) == armor::wake_local::Source::kLocal);
  Chain other;
  FunctionModel missing(76 * 32, 96, [](const float*, float*) { return true; }, false);
  Detector not_loaded(&missing, &other.classifier);
  CHECK(armor::wake_local::choose_source(true, &not_loaded, true) == armor::wake_local::Source::kService);   // no model on the board: the service is used
}

int main() {
  test_the_settings_are_checked_before_anything_runs();
  test_the_front_end_gives_one_frame_every_hop_and_keeps_less_than_a_window();
  test_silence_is_the_floor_and_a_tone_lights_the_band_of_its_pitch();
  test_the_detector_says_nothing_until_both_windows_are_full();
  test_a_tone_after_silence_is_heard_at_once_and_reset_forgets_it();
  test_a_detector_with_missing_or_wrong_models_is_not_ready_and_never_scores();
  test_a_model_that_fails_or_gives_nonsense_gives_a_score_of_zero_or_a_held_one();
  test_the_listener_wakes_on_the_second_frame_of_a_tone_and_not_on_silence();
  FINISH("wake_tflite");
}
