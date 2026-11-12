// ARMOR-NODE-HMI - host tests of the edges of the voice code: the answer of the speech service read with odd spacing and escapes, every way a multipart body, a disposition line, a
// content type, a meta part and a binary answer can be wrong, the policy that falls back to the older request, and the states of the conversation that the ordinary run never reaches.
// Stand-ins only: nothing here opens the microphone, the speaker or the network.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <cstdint>
#include <string>
#include <vector>

#include "../core/voice_answer.hpp"
#include "../core/voice_multipart.hpp"
#include "../core/voice_session.hpp"
#include "check.hpp"

using namespace armor::voice;

// ---- the answer of the speech service ------------------------------------------------------------------------------------------------------

static void test_answer_edges() {
  CHECK(panel_header_value("hmi-a1b2c3") == "hmi-a1b2c3");
  CHECK(panel_header_value("").empty() && panel_header_value("has space").empty() && panel_header_value("Bad/Id").empty());
  Answer a;
  // spacing around the colon, with tabs and line breaks, and a null audio written with spacing
  CHECK(parse_answer("{\"heard\":\"x\",\"reply\":\"y\",\"audio\" \t\r\n : \t\"QUJD\"}", a) && a.audio_base64 == "QUJD");
  CHECK(parse_answer("{\"heard\":\"x\",\"reply\":\"y\",\"audio\" :  null}", a) && a.audio_base64.empty() && a.reply == "y");
  // the key without a colon, at the end of the text or followed by something else
  CHECK(!parse_answer("{\"audio\"", a) && !parse_answer("{\"audio\" ", a) && !parse_answer("{\"audio\" 5}", a) && !parse_answer("{\"audio\":", a) && !parse_answer("{\"audio\": ", a));
  // an audio value that is not a string or null
  CHECK(!parse_answer("{\"audio\":[1]}", a) && !parse_answer("{\"audio\":true}", a) && !parse_answer("{\"audio\":nul}", a));
  // an escape that base64 does not have, and a backslash at the very end
  CHECK(!parse_answer("{\"audio\":\"AB\\nC\"}", a) && !parse_answer("{\"audio\":\"AB\\u0041\"}", a) && !parse_answer("{\"audio\":\"AB\\", a));
  // an escaped quote before the word audio inside the words (it is no key), and then the real key
  CHECK(parse_answer("{\"heard\":\"say \\\"audio\",\"reply\":\"ok\",\"audio\":\"QQ==\"}", a) && a.audio_base64 == "QQ==" && a.heard == "say \"audio");
  // only the escaped one: there is no key, so no audio at all
  CHECK(parse_answer("{\"heard\":\"say \\\"audio\",\"reply\":\"ok\"}", a) && a.audio_base64.empty() && a.reply == "ok");
  // the words missing give empty words; a document that is not an object is no answer
  CHECK(parse_answer("{}", a) && a.heard.empty() && a.reply.empty() && a.challenge.empty());
  CHECK(!parse_answer("5", a) && !parse_answer("\"text\"", a) && !parse_answer("null", a));
}

// ---- multipart ------------------------------------------------------------------------------------------------------------------------------

static const char* kMeta = "{\"language\":\"es\",\"encoding\":\"pcm_s16le\",\"sample_rate\":16000,\"channels\":1}";

static std::string part(const std::string& headers, const std::string& data) { return (headers.empty() ? std::string("\r\n") : headers + "\r\n\r\n") + data; }
static std::string disposition(const std::string& name, const char* kind = "form-data") { return std::string("Content-Disposition: ") + kind + "; name=\"" + name + "\""; }
static std::string assemble(const std::string& boundary, const std::vector<std::string>& parts, const std::string& end = "--\r\n") {
  std::string out = "--" + boundary + "\r\n" + parts[0];
  for (std::size_t i = 1; i < parts.size(); ++i) out += "\r\n--" + boundary + "\r\n" + parts[i];
  return out + "\r\n--" + boundary + end;
}
static const std::string kForm = "multipart/form-data; boundary=BND";
static std::string good_audio_part() { return part(disposition("audio") + "\r\nContent-Type: audio/L16;rate=16000", "abcd"); }
static std::string good_meta_part(const std::string& json = kMeta) { return part(disposition("meta") + "\r\nContent-Type: application/json", json); }

static bool request_ok(const std::string& body, RequestRead& out, Refusal& why) { return parse_request(kForm, body, 1000, out, why); }

static void test_disposition() {
  std::string name;
  CHECK(detail::read_disposition("form-data; name=\"meta\"", Framing::kFormData, name) && name == "meta");
  CHECK(detail::read_disposition("  FORM-DATA ;  name=\"audio\"  ", Framing::kFormData, name) && name == "audio");
  CHECK(detail::read_disposition("form-data; name = \"audio\"", Framing::kFormData, name) && name == "audio");
  CHECK(detail::read_disposition("form-data; name\t=\t\"audio\"", Framing::kFormData, name) && name == "audio");
  CHECK(detail::read_disposition("inline; name=\"result\"", Framing::kMixed, name) && name == "result");
  CHECK(detail::read_disposition("attachment; name=\"audio\"", Framing::kMixed, name) && name == "audio");
  CHECK(!detail::read_disposition("form-data; name=\"audio\"", Framing::kMixed, name));
  CHECK(!detail::read_disposition("inline; name=\"audio\"", Framing::kFormData, name));
  CHECK(!detail::read_disposition("attachment; name=\"audio\"", Framing::kFormData, name));
  CHECK(!detail::read_disposition("form-data", Framing::kFormData, name));
  CHECK(!detail::read_disposition("form-data; nam=\"x\"", Framing::kFormData, name));
  CHECK(!detail::read_disposition("form-data; name", Framing::kFormData, name));
  CHECK(!detail::read_disposition("form-data; name x", Framing::kFormData, name));
  CHECK(!detail::read_disposition("form-data; name=", Framing::kFormData, name));
  CHECK(!detail::read_disposition("form-data; name= ", Framing::kFormData, name));
  CHECK(!detail::read_disposition("form-data; name=meta", Framing::kFormData, name));
  CHECK(!detail::read_disposition("form-data; name=\"meta", Framing::kFormData, name));
  CHECK(!detail::read_disposition("form-data; name=\"Meta\"", Framing::kFormData, name));
  CHECK(!detail::read_disposition("form-data; name=\"\"", Framing::kFormData, name));
  CHECK(!detail::read_disposition("form-data; name=\"" + std::string(17, 'a') + "\"", Framing::kFormData, name));
  CHECK(detail::read_disposition("form-data; name=\"" + std::string(16, 'a') + "\"", Framing::kFormData, name));
  // a file name is tolerated in a form, and only there
  CHECK(detail::read_disposition("form-data; name=\"audio\"; filename=\"a.wav\"", Framing::kFormData, name) && name == "audio");
  CHECK(detail::read_disposition("form-data; name=\"audio\"; filename=\"\"", Framing::kFormData, name));
  CHECK(!detail::read_disposition("inline; name=\"audio\"; filename=\"a.wav\"", Framing::kMixed, name));
  CHECK(!detail::read_disposition("form-data; name=\"audio\" filename=\"a.wav\"", Framing::kFormData, name));
  CHECK(!detail::read_disposition("form-data; name=\"audio\"; other=\"a.wav\"", Framing::kFormData, name));
  CHECK(!detail::read_disposition("form-data; name=\"audio\"; filename=a.wav", Framing::kFormData, name));
  CHECK(!detail::read_disposition("form-data; name=\"audio\"; filename=\"", Framing::kFormData, name));
  CHECK(!detail::read_disposition("form-data; name=\"audio\"; filename=\"abc", Framing::kFormData, name));
  CHECK(!detail::read_disposition("form-data; name=\"audio\"; filename=\"abc\" junk", Framing::kFormData, name));
  CHECK(!detail::read_disposition("form-data; name=\"audio\"; filename=\"" + std::string(65, 'a') + "\"", Framing::kFormData, name));
  CHECK(detail::read_disposition("form-data; name=\"audio\"; filename=\"" + std::string(64, 'a') + "\"", Framing::kFormData, name));
}

static void test_boundary() {
  Refusal why;
  std::string boundary;
  CHECK(boundary_of("multipart/form-data; boundary=abc", Framing::kFormData, boundary, why) && boundary == "abc");
  CHECK(boundary_of("multipart/form-data; boundary=\"abc\"", Framing::kFormData, boundary, why) && boundary == "abc");
  CHECK(boundary_of("MULTIPART/Form-Data ; BOUNDARY = abc", Framing::kFormData, boundary, why) && boundary == "abc");
  CHECK(boundary_of("multipart/form-data; charset=utf-8; boundary=abc", Framing::kFormData, boundary, why) && boundary == "abc");
  CHECK(boundary_of("multipart/mixed; boundary=abc", Framing::kMixed, boundary, why));
  CHECK(boundary_of("multipart/form-data; boundary='()+_,-./:=?a", Framing::kFormData, boundary, why) && boundary == "'()+_,-./:=?a");
  CHECK(boundary_of(std::string("multipart/form-data; boundary=") + std::string(70, 'z'), Framing::kFormData, boundary, why));
  const char* wrong[] = {"", "text/plain", "multipart/mixed; boundary=abc", "multipart/form-data", "multipart/form-data; boundary", "multipart/form-data; boundary=", "multipart/form-data; boundary=\"\"",
                         "multipart/form-data; boundary=abc; boundary=def", "multipart/form-data; boundary=abc.", "multipart/form-data; boundary=a b", "multipart/form-data; boundary=a\"b",
                         "multipart/form-data; boundary=a;b=c;boundary=d", "multipart/form-data; charset=utf-8"};
  for (const char* type : wrong) {
    why = Refusal{};
    CHECK(!boundary_of(type, Framing::kFormData, boundary, why) && why.status == 400 && why.code == "bad_multipart");
  }
  CHECK(!boundary_of(std::string("multipart/form-data; boundary=") + std::string(71, 'z'), Framing::kFormData, boundary, why));
  CHECK(!boundary_of("multipart/form-data; boundary=abc", Framing::kMixed, boundary, why));
}

static void test_split_edges() {
  std::vector<Part> parts;
  Refusal why;
  const std::vector<std::string> allowed = {"meta", "audio"};
  auto split = [&](const std::string& body) { why = Refusal{}; return split_parts(body, "BND", Framing::kFormData, allowed, 100, 50, parts, why); };
  CHECK(split(assemble("BND", {good_meta_part("{}"), part(disposition("audio"), "ab")})) && parts.size() == 2 && parts[1].name == "audio" && parts[1].data == "ab" && parts[0].content_type == "application/json");
  CHECK(split(assemble("BND", {part(disposition("audio"), "")})) && parts.size() == 1 && parts[0].data.empty());
  // the end of the body: nothing, the line break, or something else
  CHECK(split(assemble("BND", {part(disposition("audio"), "ab")}, "--")));
  CHECK(!split(assemble("BND", {part(disposition("audio"), "ab")}, "--junk")) && why.code == "bad_multipart");
  CHECK(!split(assemble("BND", {part(disposition("audio"), "ab")}, "xx")) && why.code == "bad_multipart");
  CHECK(!split(assemble("BND", {part(disposition("audio"), "ab")}, "")) && why.code == "bad_multipart");
  // the opening and the headers
  CHECK(!split("") && !split("--BND") && !split("--BNDX\r\n") && !split("junk") && why.code == "bad_multipart");
  CHECK(!split("--BND\r\n" + disposition("audio")) && why.code == "bad_multipart");
  CHECK(!split("--BND\r\n" + disposition("audio") + "\r\n\r\nab") && why.code == "bad_multipart");   // no closing delimiter
  CHECK(!split(assemble("BND", {part("X-Pad: " + std::string(600, 'a') + "\r\n" + disposition("audio"), "ab")})) && why.code == "bad_multipart");
  CHECK(!split(assemble("BND", {part("", "ab")})) && why.code == "bad_multipart");                                                             // no headers, so no name
  CHECK(!split(assemble("BND", {part(std::string("Content-Disposition: form-data; name=\"audio\"; filename=\"caf\xC3\xA9\""), "ab")})) && why.code == "bad_multipart");
  CHECK(!split(assemble("BND", {part("no colon in this line", "ab")})) && why.code == "bad_multipart");
  CHECK(!split(assemble("BND", {part(disposition("audio") + "\r\nno colon", "ab")})) && why.code == "bad_multipart");
  CHECK(!split(assemble("BND", {part(disposition("audio") + "\r\nContent-Type: a/b\r\nContent-Type: a/b", "ab")})) && why.code == "bad_multipart");
  CHECK(!split(assemble("BND", {part(disposition("audio") + "\r\nX-Other: 1", "ab")})) && why.code == "bad_multipart");
  CHECK(!split(assemble("BND", {part(disposition("audio") + "\r\n" + disposition("meta"), "ab")})) && why.code == "bad_multipart");
  CHECK(!split(assemble("BND", {part("Content-Disposition: attachment; name=\"audio\"", "ab")})) && why.code == "bad_multipart");
  CHECK(!split(assemble("BND", {part("Content-Type: audio/L16", "ab")})) && why.code == "bad_multipart");
  CHECK(split(assemble("BND", {part("content-type: audio/L16\r\nCONTENT-DISPOSITION: form-data; name=\"audio\"", "ab")})) && parts[0].content_type == "audio/L16");
  // the names and the sizes
  CHECK(!split(assemble("BND", {part(disposition("other"), "ab")})) && why.status == 422 && why.code == "unknown_part");
  CHECK(!split(assemble("BND", {part(disposition("audio"), "ab"), part(disposition("audio"), "cd")})) && why.status == 422 && why.code == "duplicate_part");
  CHECK(!split(assemble("BND", {part(disposition("audio"), std::string(101, 'a'))})) && why.status == 413 && why.code == "audio_too_large");
  CHECK(split(assemble("BND", {part(disposition("audio"), std::string(100, 'a'))})));
  CHECK(!split(assemble("BND", {part(disposition("meta"), std::string(51, 'a'))})) && why.status == 422 && why.code == "bad_meta");
  CHECK(split(assemble("BND", {part(disposition("meta"), std::string(50, 'a'))})));
}

static void test_request_edges() {
  RequestRead read;
  Refusal why;
  CHECK(request_ok(assemble("BND", {good_meta_part(), good_audio_part()}), read, why) && read.audio == "abcd" && read.meta.present && read.meta.language == "es" && read.meta.reply_audio == ReplyAudio::kBase64);
  CHECK(request_ok(assemble("BND", {good_audio_part()}), read, why) && !read.meta.present && read.meta.language == "en");
  CHECK(!parse_request("text/plain", "x", 1000, read, why) && why.code == "bad_multipart");
  CHECK(!parse_request(kForm, std::string(6000, 'x'), 10, read, why) && why.status == 413 && why.code == "audio_too_large");
  CHECK(!parse_request(kForm, "garbage", 1000, read, why) && why.code == "bad_multipart");
  CHECK(!request_ok(assemble("BND", {good_meta_part()}), read, why) && why.status == 422 && why.code == "missing_audio");
  // the type of the audio
  for (const char* type : {"audio/L16;rate=16000", "audio/l16; rate=16000", "AUDIO/L16 ; RATE = 16000", "application/octet-stream"}) {
    CHECK(request_ok(assemble("BND", {part(disposition("audio") + "\r\nContent-Type: " + type, "abcd")}), read, why));
  }
  CHECK(request_ok(assemble("BND", {part(disposition("audio"), "abcd")}), read, why));
  CHECK(!request_ok(assemble("BND", {part(disposition("audio") + "\r\nContent-Type: audio/wav", "abcd")}), read, why) && why.status == 415 && why.code == "unsupported_audio_type");
  CHECK(!request_ok(assemble("BND", {part(disposition("audio") + "\r\nContent-Type: audio/L16;rate=8000", "abcd")}), read, why) && why.status == 415);
  // the length of the audio
  CHECK(!request_ok(assemble("BND", {part(disposition("audio"), "")}), read, why) && why.status == 411);
  CHECK(!request_ok(assemble("BND", {part(disposition("audio"), "abc")}), read, why) && why.status == 411);
  // the type and the text of the meta
  CHECK(request_ok(assemble("BND", {part(disposition("meta") + "\r\nContent-Type: Application/JSON; charset=utf-8", kMeta), good_audio_part()}), read, why));
  CHECK(request_ok(assemble("BND", {part(disposition("meta"), kMeta), good_audio_part()}), read, why));
  CHECK(!request_ok(assemble("BND", {part(disposition("meta") + "\r\nContent-Type: text/plain", kMeta), good_audio_part()}), read, why) && why.status == 422 && why.code == "bad_meta");
  CHECK(!request_ok(assemble("BND", {good_meta_part("{not json"), good_audio_part()}), read, why) && why.code == "bad_meta");
}

static void test_meta_edges() {
  Meta meta;
  auto json = [](const std::string& language, const std::string& encoding, const std::string& rate, const std::string& channels, const std::string& extra = "") {
    return "{\"language\":" + language + ",\"encoding\":" + encoding + ",\"sample_rate\":" + rate + ",\"channels\":" + channels + extra + "}";
  };
  CHECK(read_meta(json("\"en\"", "\"pcm_s16le\"", "16000", "1"), meta) && meta.present && meta.language == "en" && meta.reply_audio == ReplyAudio::kBase64);
  for (const char* language : {"en", "es", "de", "fr", "it", "ja", "zh", "pt"}) CHECK(read_meta(json(std::string("\"") + language + "\"", "\"pcm_s16le\"", "16000", "1"), meta) && meta.language == language);
  CHECK(read_meta(json("\"pt\"", "\"pcm_s16le\"", "16000", "1", ",\"reply_audio\":\"binary\""), meta) && meta.reply_audio == ReplyAudio::kBinary);
  CHECK(read_meta(json("\"pt\"", "\"pcm_s16le\"", "16000", "1", ",\"reply_audio\":\"none\""), meta) && meta.reply_audio == ReplyAudio::kNone);
  CHECK(read_meta(json("\"pt\"", "\"pcm_s16le\"", "16000", "1", ",\"reply_audio\":\"base64\""), meta) && meta.reply_audio == ReplyAudio::kBase64);
  CHECK(!read_meta(json("\"pt\"", "\"pcm_s16le\"", "16000", "1", ",\"reply_audio\":\"wav\""), meta));
  CHECK(!read_meta(json("\"pt\"", "\"pcm_s16le\"", "16000", "1", ",\"reply_audio\":7"), meta));
  CHECK(!read_meta(json("\"pt\"", "\"pcm_s16le\"", "16000", "1", ",\"extra\":1"), meta));
  CHECK(!read_meta(json("\"xx\"", "\"pcm_s16le\"", "16000", "1"), meta) && !read_meta(json("5", "\"pcm_s16le\"", "16000", "1"), meta));
  CHECK(!read_meta(json("\"en\"", "\"mp3\"", "16000", "1"), meta) && !read_meta(json("\"en\"", "5", "16000", "1"), meta));
  CHECK(!read_meta(json("\"en\"", "\"pcm_s16le\"", "8000", "1"), meta) && !read_meta(json("\"en\"", "\"pcm_s16le\"", "\"16000\"", "1"), meta));
  CHECK(!read_meta(json("\"en\"", "\"pcm_s16le\"", "16000", "2"), meta) && !read_meta(json("\"en\"", "\"pcm_s16le\"", "16000", "\"1\""), meta));
  CHECK(!read_meta("{\"encoding\":\"pcm_s16le\",\"sample_rate\":16000,\"channels\":1}", meta) && !read_meta("{\"language\":\"en\",\"sample_rate\":16000,\"channels\":1}", meta));
  CHECK(!read_meta("{\"language\":\"en\",\"encoding\":\"pcm_s16le\",\"channels\":1}", meta) && !read_meta("{\"language\":\"en\",\"encoding\":\"pcm_s16le\",\"sample_rate\":16000}", meta));
  CHECK(!read_meta("[]", meta) && !read_meta("", meta) && !read_meta("5", meta));
}

static std::string result_part(const std::string& json) { return part(disposition("result", "inline") + "\r\nContent-Type: application/json", json); }
static std::string audio_part(const std::string& data) { return part(disposition("audio", "inline") + "\r\nContent-Type: audio/L16;rate=16000", data); }

static void test_reply_edges() {
  const std::string type = "multipart/mixed; boundary=BND";
  BinaryAnswer answer;
  Refusal why;
  auto reply = [&](const std::string& body) { why = Refusal{}; return parse_reply(type, body, 1000, answer, why); };
  CHECK(reply(assemble("BND", {result_part("{\"heard\":\"h\",\"reply\":\"r\"}")})) && answer.heard == "h" && answer.reply == "r" && answer.audio.empty() && !answer.audio_omitted && answer.challenge.empty());
  CHECK(reply(assemble("BND", {result_part("{\"heard\":\"h\",\"reply\":\"r\"}"), audio_part("abcd")})) && answer.audio == "abcd");
  CHECK(reply(assemble("BND", {result_part("{\"heard\":\"h\",\"reply\":\"r\",\"challenge\":\"4711\"}")})) && answer.challenge == "4711");
  CHECK(reply(assemble("BND", {result_part("{\"heard\":\"h\",\"reply\":\"r\",\"audio_omitted\":\"too_large\"}")})) && answer.audio_omitted);
  CHECK(!parse_reply("multipart/form-data; boundary=BND", "x", 1000, answer, why) && why.code == "bad_multipart");
  CHECK(!reply("junk") && why.code == "bad_multipart");
  CHECK(!reply(assemble("BND", {audio_part("abcd")})) && why.status == 422 && why.code == "bad_meta");
  CHECK(!reply(assemble("BND", {result_part("not json")})) && why.code == "bad_meta");
  CHECK(!reply(assemble("BND", {result_part("[]")})) && why.code == "bad_meta");
  CHECK(!reply(assemble("BND", {result_part("{\"heard\":\"h\",\"reply\":\"r\",\"audio\":\"QQ==\"}")})) && why.code == "bad_meta");
  CHECK(!reply(assemble("BND", {result_part("{\"heard\":\"h\",\"reply\":\"r\",\"extra\":1}")})) && why.code == "bad_meta");
  CHECK(!reply(assemble("BND", {result_part("{\"reply\":\"r\"}")})) && !reply(assemble("BND", {result_part("{\"heard\":\"h\"}")})));
  CHECK(!reply(assemble("BND", {result_part("{\"heard\":1,\"reply\":\"r\"}")})) && !reply(assemble("BND", {result_part("{\"heard\":\"h\",\"reply\":2}")})));
  CHECK(!reply(assemble("BND", {result_part("{\"heard\":\"h\",\"reply\":\"r\",\"challenge\":47}")})) && why.code == "bad_meta");
  CHECK(!reply(assemble("BND", {result_part("{\"heard\":\"h\",\"reply\":\"r\",\"challenge\":\"4\"}")})) && why.code == "bad_meta");
  CHECK(!reply(assemble("BND", {result_part("{\"heard\":\"h\",\"reply\":\"r\",\"audio_omitted\":true}")})) && why.code == "bad_meta");
  CHECK(!reply(assemble("BND", {result_part("{\"heard\":\"h\",\"reply\":\"r\",\"audio_omitted\":\"other\"}")})) && why.code == "bad_meta");
  CHECK(!reply(assemble("BND", {result_part("{\"heard\":\"h\",\"reply\":\"r\"}"), audio_part("")})) && why.status == 411);
  CHECK(!reply(assemble("BND", {result_part("{\"heard\":\"h\",\"reply\":\"r\"}"), audio_part("abc")})) && why.status == 411);
}

static void test_framing_and_policy() {
  CHECK(make_boundary(1, 2, 3) == "armor000000010000000200000003" && make_boundary(0xFFFFFFFFu, 0, 0xABCDEF01u) == "armorffffffff00000000abcdef01");
  const std::uint8_t audio[] = {'a', 'b', '-', '-', 'B', 'N', 'D', 'c'};
  CHECK(audio_holds(audio, sizeof audio, "--BND") && !audio_holds(audio, sizeof audio, "--XYZ") && !audio_holds(audio, sizeof audio, "") && !audio_holds(audio, 2, "--BND") && !audio_holds(audio, 0, "x"));
  CHECK(std::string(to_text(ReplyAudio::kBase64)) == "base64" && std::string(to_text(ReplyAudio::kBinary)) == "binary" && std::string(to_text(ReplyAudio::kNone)) == "none");
  RequestFraming framing;
  CHECK(!build_request_framing("", "en", ReplyAudio::kBase64, framing) && !build_request_framing("bad boundary", "en", ReplyAudio::kBase64, framing) && !build_request_framing("ok", "xx", ReplyAudio::kBase64, framing));
  for (ReplyAudio mode : {ReplyAudio::kBase64, ReplyAudio::kBinary, ReplyAudio::kNone}) {
    CHECK(build_request_framing("BND", "de", mode, framing) && framing.content_type == "multipart/form-data; boundary=BND");
    // what it builds is read back by the reader of the same file
    RequestRead read;
    Refusal why;
    CHECK(parse_request(framing.content_type, framing.head + "abcd" + framing.tail, 1000, read, why) && read.audio == "abcd" && read.meta.language == "de" && read.meta.reply_audio == mode);
  }
  BinaryTurnPolicy policy;
  CHECK(policy.should_try(0));
  policy.on_refused(1000);
  CHECK(!policy.should_try(1000) && !policy.should_try(1000 + BinaryTurnPolicy::kHoldMs - 1) && policy.should_try(1000 + BinaryTurnPolicy::kHoldMs));
  policy.on_refused(5000);
  policy.on_accepted();
  CHECK(policy.should_try(5001));
  policy.on_refused(9000);
  policy.set_allowed(false);
  CHECK(!policy.should_try(9001));
  policy.set_allowed(true);
  CHECK(policy.should_try(9001));      // turning it off forgot the refusal
  policy.on_refused(9500);
  policy.set_allowed(true);
  CHECK(!policy.should_try(9501));     // turning it on again keeps a refusal that is still held
  BinaryTurnPolicy off(false);
  CHECK(!off.should_try(0));
  CHECK(BinaryTurnPolicy::refuses_binary(415) && BinaryTurnPolicy::refuses_binary(400) && BinaryTurnPolicy::refuses_binary(422) && !BinaryTurnPolicy::refuses_binary(200) && !BinaryTurnPolicy::refuses_binary(500));
}

// ---- the conversation -----------------------------------------------------------------------------------------------------------------------

static void test_session_edges() {
  Session s;
  s.set_enabled(false, 0);
  CHECK(s.state() == State::kOff);
  s.set_enabled(true, 100);
  CHECK(s.state() == State::kIdle);
  s.set_enabled(true, 5000);                       // already on: nothing changes
  CHECK(s.state() == State::kIdle);
  CHECK(s.on_event(Event::kFailure, 6000, "x") == Action::kNone && s.on_event(Event::kTouch, 6000) == Action::kNone);
  CHECK(s.on_event(Event::kSpeechEnded, 6000) == Action::kNone && s.on_event(Event::kReply, 6000) == Action::kNone && s.on_event(Event::kSpeechDone, 6000) == Action::kNone);
  CHECK(s.on_event(static_cast<Event>(99), 6000) == Action::kNone);
  CHECK(s.on_event(Event::kWake, 6000) == Action::kStartRecording);
  CHECK(s.on_event(Event::kWake, 6001) == Action::kNone);               // already listening
  CHECK(s.on_event(Event::kReply, 6002) == Action::kNone && s.on_event(Event::kSpeechDone, 6002) == Action::kNone);
  CHECK(s.on_event(Event::kFailure, 6100) == Action::kShowFailure && s.failure() == "failed");   // no reason given
  CHECK(s.on_event(Event::kFailure, 6200, "again") == Action::kNone && s.failure() == "failed");   // already failed
  CHECK(s.on_event(Event::kTouch, 6300) == Action::kBackToIdle && s.state() == State::kIdle);
  CHECK(s.on_event(Event::kWake, 6400) == Action::kNone);               // the cooldown after a conversation
  CHECK(s.on_event(Event::kWake, 6300 + 1500) == Action::kStartRecording);
  CHECK(s.on_event(Event::kSpeechEnded, 8000) == Action::kStopRecordingAndSend);
  CHECK(s.on_event(Event::kSpeechEnded, 8001) == Action::kNone);
  CHECK(s.on_event(Event::kFailure, 8100, "no network") == Action::kShowFailure && s.failure() == "no network");
  CHECK(s.tick(8100 + 3999) == Action::kNone && s.state() == State::kFailed);
  CHECK(s.tick(8100 + 4000) == Action::kBackToIdle && s.state() == State::kIdle);
  CHECK(s.tick(9000) == Action::kNone);
  // thinking times out, speaking is cut at its limit, a touch stops the audio, the microphone is closed at its limit
  CHECK(s.on_event(Event::kWake, 20000) == Action::kStartRecording);
  CHECK(s.tick(20000 + 7999) == Action::kNone && s.tick(20000 + 8000) == Action::kStopRecordingAndSend && s.state() == State::kThinking);
  CHECK(s.tick(28000 + 14999) == Action::kNone && s.tick(28000 + 15000) == Action::kShowFailure && s.failure() == "timeout");
  CHECK(s.on_event(Event::kTouch, 44000) == Action::kBackToIdle);
  CHECK(s.on_event(Event::kWake, 50000) == Action::kStartRecording && s.on_event(Event::kSpeechEnded, 50100) == Action::kStopRecordingAndSend && s.on_event(Event::kReply, 50200) == Action::kStartSpeaking);
  CHECK(s.still(State::kSpeaking, 50300) && !s.still(State::kThinking, 50300));
  CHECK(s.tick(50200 + 29999) == Action::kNone && s.tick(50200 + 30000) == Action::kStopAudio && s.state() == State::kIdle);
  CHECK(s.on_event(Event::kWake, 90000) == Action::kStartRecording && s.on_event(Event::kSpeechEnded, 90100) == Action::kStopRecordingAndSend && s.on_event(Event::kReply, 90200) == Action::kStartSpeaking);
  CHECK(s.on_event(Event::kTouch, 90300) == Action::kStopAudio && s.state() == State::kIdle);
  CHECK(s.on_event(Event::kWake, 95000) == Action::kStartRecording && s.on_event(Event::kTouch, 95100) == Action::kStopAudio);
  // switching it off in the middle of a conversation ends it
  CHECK(s.on_event(Event::kWake, 99000) == Action::kStartRecording);
  s.set_enabled(false, 99100);
  CHECK(s.state() == State::kOff && s.on_event(Event::kWake, 99200) == Action::kNone && s.tick(100000) == Action::kNone);
  // a shorter microphone window
  s.set_enabled(true, 100000);
  s.set_listen_ms(500);
  CHECK(s.on_event(Event::kWake, 110000) == Action::kStartRecording && s.tick(110499) == Action::kNone && s.tick(110500) == Action::kStopRecordingAndSend);
  CHECK(std::string(to_text(State::kOff)) == "off" && std::string(to_text(State::kIdle)) == "idle" && std::string(to_text(State::kListening)) == "listening" && std::string(to_text(State::kThinking)) == "thinking" &&
        std::string(to_text(State::kSpeaking)) == "speaking" && std::string(to_text(State::kFailed)) == "failed" && std::string(to_text(static_cast<State>(99))) == "off");
}

int main() {
  test_answer_edges();
  test_disposition();
  test_boundary();
  test_split_edges();
  test_request_edges();
  test_meta_edges();
  test_reply_edges();
  test_framing_and_policy();
  test_session_edges();
  FINISH("test_voice_edges");
}
