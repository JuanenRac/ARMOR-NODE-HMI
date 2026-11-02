// ARMOR-NODE-HMI - host tests of acknowledging the alarms from the panel (core/ack_flow.hpp): what a touch asks first, the body that is sent, what each answer of the server means, the setting that
// chooses it (controls.ack) and the agreement with the server's contract (the OpenAPI of ARMOR-COMMON and the route of ARMOR-SERVER, where the sibling repositories are next to this one).
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
// Usage: test_ack_flow [ARMOR-COMMON folder] [ARMOR-SERVER folder]
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>

#include "../core/ack_flow.hpp"
#include "../core/hmi_config.hpp"

static int failures = 0;
static int checks = 0;
#define CHECK(condition)                                                              \
  do {                                                                                \
    ++checks;                                                                         \
    if (!(condition)) {                                                               \
      ++failures;                                                                     \
      std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #condition);                \
    }                                                                                 \
  } while (0)

using namespace armor;
using hmi::AckMode;
using hmi::AckStep;

static std::string read_file(const std::string& path) {
  std::ifstream file(path, std::ios::binary);
  return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

static void test_what_a_touch_asks() {
  CHECK(hmi::ack_step(AckMode::kPin) == AckStep::kAskForPin && hmi::ack_step(AckMode::kConfirm) == AckStep::kAskToConfirm && hmi::ack_step(AckMode::kOff) == AckStep::kSendNow);
  for (const AckMode mode : {AckMode::kPin, AckMode::kConfirm, AckMode::kOff}) { AckMode back = AckMode::kOff; CHECK(hmi::parse_ack_mode(hmi::ack_mode_text(mode), back) && back == mode); }
  AckMode none = AckMode::kPin;
  CHECK(!hmi::parse_ack_mode("", none) && !hmi::parse_ack_mode("PIN", none) && !hmi::parse_ack_mode("never", none) && none == AckMode::kPin);
}

static void test_the_body() {
  CHECK(hmi::ack_body("") == "{}");
  CHECK(hmi::ack_body("1234") == "{\"pin\":\"1234\"}");
  CHECK(hmi::ack_body("0042") == "{\"pin\":\"0042\"}");   // a leading zero is a digit
  CHECK(hmi::ack_body("12\"34") == "{\"pin\":\"1234\"}");   // nothing that could break the JSON gets in
  CHECK(hmi::ack_body("ab\\c") == "{}" && hmi::ack_body("\n ") == "{}");
  hmi::PinEntry typed;
  for (const char c : std::string("9876x5")) typed.add(c);   // the keypad only takes digits, and no more than ten
  CHECK(typed.digits() == "98765" && hmi::ack_body(typed.digits()) == "{\"pin\":\"98765\"}");
  hmi::PinEntry many;
  for (int i = 0; i < 20; ++i) many.add('7');
  CHECK(many.digits().size() == hmi::PinEntry::kMaxDigits && hmi::ack_body(many.digits()).size() == 8 + hmi::PinEntry::kMaxDigits + 2);
}

static void test_what_an_answer_means() {
  CHECK(hmi::order_error(200, "{\"acknowledged\":3}").empty());
  CHECK(hmi::order_error(403, "{\"error\":\"the PIN is not right\",\"code\":\"wrong_pin\"}") == "wrong_pin");
  CHECK(hmi::order_error(429, "{\"error\":\"too many wrong PINs; try again later\",\"code\":\"pin_locked\",\"retry_after_s\":30}") == "pin_locked");
  CHECK(hmi::order_error(403, "{\"error\":\"no PIN\",\"code\":\"pin_not_set\"}") == "pin_not_set");
  CHECK(hmi::order_error(403, "{\"error\":\"not for a panel\",\"code\":\"forbidden\"}") == "forbidden");   // a 403 that is not about the PIN
  CHECK(hmi::order_error(403, "") == "forbidden");
  CHECK(hmi::order_error(0, "") == "no_answer");
  CHECK(hmi::order_error(401, "{}") == "unauthorized");
  CHECK(hmi::order_error(500, "boom") == "failed" && hmi::order_error(404, "{}") == "failed" && hmi::order_error(429, "{}") == "failed" && hmi::order_error(400, "{\"code\":\"wrong_pin\"}") == "failed");
  // a PIN error the server did not send with the status it uses is not taken for one
  CHECK(hmi::order_error(200, "{\"code\":\"wrong_pin\"}").empty());
}

static void test_the_setting() {
  config::Settings s = config::default_settings("a1b2c3");
  s.sta.enabled = true; s.sta.ssid = "casa"; s.sta.password = "una-clave-larga";
  CHECK(s.controls.ack == AckMode::kPin);   // the safe way until the owner says otherwise
  const std::string text = config::to_json(s, true);
  CHECK(text.find("\"controls\":{\"ack\":\"pin\"}") != std::string::npos);
  config::Problems problems;
  config::Settings back;
  CHECK(config::load("{\"controls\":{\"ack\":\"confirm\"}}", s, back, problems) && back.controls.ack == AckMode::kConfirm && config::to_json(back, true).find("\"controls\":{\"ack\":\"confirm\"}") != std::string::npos);
  problems.clear();
  CHECK(config::load("{\"controls\":{\"ack\":\"off\"}}", s, back, problems) && back.controls.ack == AckMode::kOff);
  problems.clear();
  config::Settings bad;
  CHECK(!config::load("{\"controls\":{\"ack\":\"sometimes\"}}", s, bad, problems) && !problems.empty() && problems[0].path == "controls.ack" && problems[0].code == "invalid");
  problems.clear();
  CHECK(!config::load("{\"controls\":{\"ack\":1}}", s, bad, problems) && problems[0].path == "controls.ack");
  // a document without it leaves it as it was
  problems.clear();
  config::Settings off = back;
  CHECK(config::load("{\"display\":{}}", off, bad, problems) && bad.controls.ack == AckMode::kOff);
}

// The server's side of the contract, where the sibling repositories are present: the order exists, takes a PIN and answers with the codes the panel understands.
static void test_the_contract(const std::string& common, const std::string& server) {
  if (common.empty()) { std::printf("ARMOR-COMMON not given: the contract with the server was not compared\n"); return; }
  const std::string openapi = read_file(common + "/openapi/armor-server.yaml");
  if (openapi.empty()) { std::printf("the OpenAPI of ARMOR-COMMON was not found at %s: the contract with the server was not compared\n", common.c_str()); return; }
  const std::size_t at = openapi.find("/api/v1/alarms/acknowledge:");
  CHECK(at != std::string::npos);
  const std::size_t next = openapi.find("\n  /api/v1/", at + 10);
  const std::string route = openapi.substr(at, next == std::string::npos ? std::string::npos : next - at);
  CHECK(route.find("PostAlarmsAcknowledgeRequest") != std::string::npos && route.find("'403'") != std::string::npos && route.find("'429'") != std::string::npos && route.find("'200'") != std::string::npos);
  const std::size_t schema = openapi.find("    PostAlarmsAcknowledgeRequest:");
  CHECK(schema != std::string::npos);
  if (schema != std::string::npos) {
    const std::string body = openapi.substr(schema, 500);
    CHECK(body.find("pin: { type: string, maxLength: 12") != std::string::npos && hmi::PinEntry::kMaxDigits <= 12);   // the panel never sends a longer PIN than the server takes
  }
  if (server.empty()) return;
  const std::string route_source = read_file(server + "/src/routes/alarms.ts");
  if (route_source.empty()) { std::printf("ARMOR-SERVER was not found at %s: its route was not compared\n", server.c_str()); return; }
  for (const char* code : {"wrong_pin", "pin_locked"}) CHECK(route_source.find(std::string("code: \"") + code + "\"") != std::string::npos);
  CHECK(route_source.find("/api/v1/alarms/acknowledge") != std::string::npos && route_source.find("status(403)") != std::string::npos && route_source.find("status(429)") != std::string::npos);
}

int main(int argc, char** argv) {
  test_what_a_touch_asks();
  test_the_body();
  test_what_an_answer_means();
  test_the_setting();
  test_the_contract(argc > 1 ? argv[1] : "", argc > 2 ? argv[2] : "");
  std::printf("%d checks, %d failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}
