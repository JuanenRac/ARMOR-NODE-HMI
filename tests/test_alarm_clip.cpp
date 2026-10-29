// ARMOR-NODE-HMI - host tests of the audio the server makes for an alarm (core/alarm_clip.hpp, core/sha256.hpp): what is asked for, what is checked before it is played, what the panel
// remembers and when the audio replaces the tone.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <string>
#include <vector>

#include "../core/alarm_clip.hpp"
#include "check.hpp"

using namespace armor;

static std::string pcm(std::size_t bytes, char fill = 7) { return std::string(bytes, fill); }

static hmi::AlarmLine alarm(const char* source, hmi::Severity severity, const char* code = "electrical_grid_lost") {
  hmi::AlarmLine line;
  line.id = "a";
  line.source_type = source;
  line.severity = severity;
  line.code = code;
  return line;
}

int main() {
  // SHA-256 against the published vectors, in the sizes that are the edges of its blocks
  CHECK(sha256::hex_of("") == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
  CHECK(sha256::hex_of("abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
  CHECK(sha256::hex_of("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") == "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
  CHECK(sha256::hex_of(std::string(1000000, 'a')) == "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
  CHECK(sha256::hex_of(std::string(55, 'a')) != sha256::hex_of(std::string(56, 'a')) && sha256::hex_of(std::string(64, 'a')).size() == 64);

  // what is asked for: only closed lists and plain characters reach the address
  clip::Request spoken;
  spoken.source = hmi::source_kind("electrical");
  spoken.severity = hmi::Severity::kCritical;
  spoken.code = "electrical_grid_lost";
  CHECK(clip::path_for(spoken, "es") == "/api/v1/alarm-audio/spoken?language=es&source=electrical&severity=critical&code=electrical_grid_lost");
  CHECK(clip::path_for(spoken, "../etc") == "/api/v1/alarm-audio/spoken?language=en&source=electrical&severity=critical&code=electrical_grid_lost");   // a language that is not one of the eight
  spoken.code = "A B&x=1";
  CHECK(clip::path_for(spoken, "en") == "/api/v1/alarm-audio/spoken?language=en&source=electrical&severity=critical");
  CHECK(clip::plain_code("solar_fault") == "solar_fault" && clip::plain_code("").empty() && clip::plain_code("_x").empty() && clip::plain_code("a/b").empty() && clip::plain_code(std::string(65, 'a')).empty() && clip::plain_code("Abc").empty());
  clip::Request custom;
  custom.kind = clip::Kind::kCustom;
  custom.source = hmi::source_kind("something else");
  CHECK(clip::path_for(custom, "en") == "/api/v1/alarm-audio/sounds/other");
  custom.source = hmi::source_kind("network");
  CHECK(clip::path_for(custom, "de") == "/api/v1/alarm-audio/sounds/network");

  // what is checked before playing
  const std::string good = pcm(6400);
  const std::string hash = sha256::hex_of(good);
  CHECK(clip::check(good, hash, 200000) == clip::Check::kOk);
  CHECK(clip::check("", hash, 200000) == clip::Check::kEmpty);
  CHECK(clip::check(pcm(3198), sha256::hex_of(pcm(3198)), 200000) == clip::Check::kTooSmall);
  CHECK(clip::check(good, hash, 6398) == clip::Check::kTooLarge);
  CHECK(clip::check(pcm(300000), sha256::hex_of(pcm(300000)), 999999999) == clip::Check::kTooLarge);   // whatever the setting says, there is a ceiling
  CHECK(clip::check(pcm(6401), sha256::hex_of(pcm(6401)), 200000) == clip::Check::kOddSize);
  CHECK(clip::check(good, "", 200000) == clip::Check::kNoHash);
  CHECK(clip::check(good, "xyz", 200000) == clip::Check::kBadHash);
  CHECK(clip::check(good, std::string(64, 'A'), 200000) == clip::Check::kBadHash);   // the server writes it in lower case
  CHECK(clip::check(good, sha256::hex_of(pcm(6400, 8)), 200000) == clip::Check::kHashMismatch);
  CHECK(std::string(clip::check_name(clip::Check::kHashMismatch)) == "hash_mismatch");
  const std::vector<std::int16_t> samples = clip::samples_of(std::string("\x01\x00\xff\xff\x00\x80", 6));
  CHECK(samples.size() == 3 && samples[0] == 1 && samples[1] == -1 && samples[2] == -32768);

  // what the panel remembers: the most recently used stay, inside the limits
  clip::Cache cache(3, 1000);
  cache.put("a", std::vector<std::int16_t>(100)); cache.put("b", std::vector<std::int16_t>(100)); cache.put("c", std::vector<std::int16_t>(100));
  CHECK(cache.find("a") != nullptr);
  cache.put("d", std::vector<std::int16_t>(100));
  CHECK(cache.find("b") == nullptr && cache.find("a") != nullptr && cache.find("c") != nullptr && cache.find("d") != nullptr && cache.size() == 3);
  cache.put("big", std::vector<std::int16_t>(501));
  CHECK(cache.find("big") == nullptr && cache.bytes() <= 1000);
  cache.put("a", std::vector<std::int16_t>(50));
  CHECK(cache.find("a")->size() == 50 && cache.bytes() == 100 * 2 * 2 + 50 * 2);

  // when the audio replaces the tone
  std::array<hmi::ToneChoice, hmi::kSourceKinds> by_source{};
  by_source[static_cast<std::size_t>(hmi::source_kind("electrical"))] = hmi::ToneChoice::kSpoken;
  by_source[static_cast<std::size_t>(hmi::source_kind("solar"))] = hmi::ToneChoice::kCustom;
  {
    const clip::Plan plan = clip::plan_sound({alarm("electrical", hmi::Severity::kCritical)}, 0, by_source);
    CHECK(plan.cue == hmi::Cue::kSiren && plan.clip.has_value() && plan.clip->kind == clip::Kind::kSpoken && plan.clip->code == "electrical_grid_lost" && plan.clip->severity == hmi::Severity::kCritical);   // the tone is the fallback
  }
  {
    const clip::Plan plan = clip::plan_sound({alarm("solar", hmi::Severity::kHigh, "solar_fault")}, 0, by_source);
    CHECK(plan.clip.has_value() && plan.clip->kind == clip::Kind::kCustom && plan.cue == hmi::Cue::kAlarm);
  }
  CHECK(!clip::plan_sound({alarm("network", hmi::Severity::kCritical)}, 0, by_source).clip.has_value());   // an origin that was not set to it
  CHECK(!clip::plan_sound({}, 0, by_source).clip.has_value());
  {
    // a siren for a critical alarm of another origin is never replaced by the audio of a lesser one
    const clip::Plan plan = clip::plan_sound({alarm("electrical", hmi::Severity::kWarning), alarm("network", hmi::Severity::kCritical)}, 0, by_source);
    CHECK(plan.cue == hmi::Cue::kSiren && !plan.clip.has_value());
  }
  {
    // the audio of an origin chosen sounds even below the least severity of the settings, like any chosen tone; the most serious one is asked for
    const clip::Plan plan = clip::plan_sound({alarm("electrical", hmi::Severity::kInfo), alarm("solar", hmi::Severity::kHigh, "solar_fault")}, 2, by_source);
    CHECK(plan.clip.has_value() && plan.clip->kind == clip::Kind::kCustom && plan.cue == hmi::Cue::kAlarm);
  }
  {
    const clip::Plan plan = clip::plan_sound({alarm("electrical", hmi::Severity::kInfo)}, 2, by_source);
    CHECK(plan.clip.has_value() && plan.cue == hmi::Cue::kNotice);
  }
  // the names of the tones that are audio from the server
  hmi::ToneChoice tone = hmi::ToneChoice::kAuto;
  CHECK(hmi::tone_from_name("spoken", tone) && tone == hmi::ToneChoice::kSpoken && hmi::tone_from_name("custom", tone) && tone == hmi::ToneChoice::kCustom && !hmi::tone_from_name("clip", tone));
  CHECK(std::string(hmi::tone_name(hmi::ToneChoice::kSpoken)) == "spoken" && std::string(hmi::tone_name(hmi::ToneChoice::kCustom)) == "custom");

  FINISH("alarm clip");
}
