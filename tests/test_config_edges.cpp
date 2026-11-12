// ARMOR-NODE-HMI - host tests of the edges of the panel's settings (core/hmi_config.hpp): every member of a received document with a value of the wrong type, too long or out of range,
// the lists of backup networks and brokers, the password that follows its host, and each rule of validate().
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
// Nothing here touches hardware or a network.
#include <cstdint>
#include <string>
#include <vector>

#include "../core/hmi_config.hpp"
#include "check.hpp"

using namespace armor;

static bool has(const config::Problems& problems, const std::string& path, const std::string& code) {
  for (const config::Problem& p : problems) if (p.path == path && p.code == code) return true;
  return false;
}

static config::Problems read(const std::string& text, config::Settings& settings) {
  config::Problems problems;
  json::Value document;
  CHECK(json::parse(text, document));
  config::read_settings(document, settings, problems);
  return problems;
}
static config::Problems read(const std::string& text) {
  config::Settings settings = config::default_settings("a1b2c3");
  return read(text, settings);
}

// ---- a member of each kind, with a value it must refuse ---------------------------------------------------------------------------

enum class Kind { kText, kSecret, kBool, kInt };
struct Field {
  Field(const char* in_section, const char* in_name, Kind in_kind, long long in_low = 0, long long in_high = 0) : section(in_section), name(in_name), kind(in_kind), low(in_low), high(in_high) {}
  const char* section;
  const char* name;
  Kind kind;
  long long low, high;   // the limits of a number (0, 0 when they are not asserted)
};

static std::string document_with(const Field& field, const std::string& value) {
  return std::string("{\"") + field.section + "\":{\"" + field.name + "\":" + value + "}}";
}

static void test_each_member_refuses_a_wrong_value() {
  const std::vector<Field> fields = {
      {"node", "id", Kind::kText}, {"node", "name", Kind::kText}, {"node", "hostname", Kind::kText},
      {"ip", "dhcp", Kind::kBool}, {"ip", "address", Kind::kText}, {"ip", "netmask", Kind::kText}, {"ip", "gateway", Kind::kText}, {"ip", "dns1", Kind::kText}, {"ip", "dns2", Kind::kText},
      {"ap", "enabled", Kind::kBool}, {"ap", "ssid", Kind::kText}, {"ap", "password", Kind::kSecret}, {"ap", "channel", Kind::kInt, 0, 13}, {"ap", "hidden", Kind::kBool},
      {"ap", "max_clients", Kind::kInt, 1, 10}, {"ap", "tx_power_dbm", Kind::kInt, 2, 20}, {"ap", "bandwidth_mhz", Kind::kInt, 20, 40}, {"ap", "country", Kind::kText},
      {"sta", "enabled", Kind::kBool}, {"sta", "ssid", Kind::kText}, {"sta", "password", Kind::kSecret},
      {"mqtt", "enabled", Kind::kBool}, {"mqtt", "uri", Kind::kText}, {"mqtt", "username", Kind::kText}, {"mqtt", "password", Kind::kSecret}, {"mqtt", "heartbeat_s", Kind::kInt, 2, 300},
      {"mqtt", "tls", Kind::kBool}, {"mqtt", "ca_pem", Kind::kText}, {"mqtt", "ntp", Kind::kText},
      {"server", "enabled", Kind::kBool}, {"server", "host", Kind::kText}, {"server", "port", Kind::kInt, 1, 65535}, {"server", "tls", Kind::kBool}, {"server", "cert_sha256", Kind::kText},
      {"server", "cert_pem", Kind::kText}, {"server", "user", Kind::kText}, {"server", "password", Kind::kSecret}, {"server", "poll_s", Kind::kInt, 1, 60}, {"server", "live_events", Kind::kBool},
      {"display", "brightness", Kind::kInt, 5, 100}, {"display", "sleep_s", Kind::kInt, 0, 3600}, {"display", "night_brightness", Kind::kInt, 1, 100},
      {"display", "night_from", Kind::kInt, 0, 23}, {"display", "night_to", Kind::kInt, 0, 23},
      {"audio", "volume", Kind::kInt, 0, 100}, {"audio", "alarm_sound", Kind::kBool}, {"audio", "critical_min_volume", Kind::kInt, 0, 100}, {"audio", "siren_repeat_s", Kind::kInt},
      {"audio", "siren_max_repeats", Kind::kInt}, {"audio", "clip_max_bytes", Kind::kInt},
      {"voice", "enabled", Kind::kBool}, {"voice", "url", Kind::kText}, {"voice", "token", Kind::kSecret}, {"voice", "wake_name", Kind::kText}, {"voice", "wake_word", Kind::kBool},
      {"voice", "binary_turn", Kind::kBool}, {"voice", "listen_s", Kind::kInt, 2, 15}, {"voice", "end_of_speech", Kind::kBool}, {"voice", "silence_ms", Kind::kInt, 300, 3000},
      {"voice", "min_speech_ms", Kind::kInt, 80, 1500}, {"voice", "level_percent", Kind::kInt, 150, 1000}, {"voice", "min_level", Kind::kInt, 50, 4000},
      {"voice", "lead_ms", Kind::kInt, 1000, 15000}, {"voice", "calibrate_ms", Kind::kInt, 100, 1000},
      {"time", "ntp_enabled", Kind::kBool}, {"time", "rtc_enabled", Kind::kBool}, {"time", "ntp", Kind::kText}, {"time", "zone", Kind::kText},
      {"ui", "language", Kind::kText},
      {"system", "auto_restart_hours", Kind::kInt, 0, 48}, {"system", "login_limit_per_minute", Kind::kInt, 0, 120},
      {"security", "seal_secrets", Kind::kBool},
  };
  const std::string long_text = "\"" + std::string(20000, 'x') + "\"";
  for (const Field& field : fields) {
    const std::string path = std::string(field.section) + "." + field.name;
    switch (field.kind) {
      case Kind::kText:
        CHECK(has(read(document_with(field, "true")), path, "invalid"));
        CHECK(has(read(document_with(field, "12")), path, "invalid"));
        CHECK(has(read(document_with(field, long_text)), path, "too_long"));
        break;
      case Kind::kSecret:
        CHECK(has(read(document_with(field, "true")), path, "invalid"));
        CHECK(has(read(document_with(field, "\"" + std::string(65, 'p') + "\"")), path, "too_long"));
        CHECK(read(document_with(field, "\"\"")).empty());   // empty keeps what is stored
        break;
      case Kind::kBool:
        CHECK(has(read(document_with(field, "\"yes\"")), path, "invalid"));
        CHECK(has(read(document_with(field, "1")), path, "invalid"));
        break;
      case Kind::kInt:
        CHECK(has(read(document_with(field, "\"7\"")), path, "invalid"));
        CHECK(has(read(document_with(field, "2000000000")), path, "range"));
        if (field.high != 0) {
          CHECK(has(read(document_with(field, std::to_string(field.high + 1))), path, "range"));
          CHECK(has(read(document_with(field, std::to_string(field.low - 1))), path, "range"));
          CHECK(read(document_with(field, std::to_string(field.high))).empty() || has(read(document_with(field, std::to_string(field.high))), path, "range") == false);
          CHECK(!has(read(document_with(field, std::to_string(field.low))), path, "range"));
        }
        break;
    }
  }
  CHECK(has(read("[]"), "", "invalid") && has(read("12"), "", "invalid"));
  CHECK(read("{\"unknown\":{\"a\":1},\"node\":5,\"ip\":[1]}").empty());   // a section that is not an object, or not known, is passed over
}

static void test_secrets_can_be_cleared_and_follow_their_host() {
  config::Settings s = config::default_settings("a1b2c3");
  s.sta.password = "wifi-password";
  s.ap.password = "ap-password";
  s.mqtt.uri = "mqtt://Broker.lan:1883";
  s.mqtt.password = "mqtt-password";
  s.server.host = "server.lan";
  s.server.password = "server-password";
  s.voice.url = "http://voice.lan:18090";
  s.voice.token = "voice-token";
  CHECK(read("{\"sta\":{\"password_clear\":true},\"ap\":{\"password_clear\":true}}", s).empty() && s.sta.password.empty() && s.ap.password.empty());
  CHECK(read("{\"mqtt\":{\"uri\":\"mqtt://broker.lan:8883\"}}", s).empty() && s.mqtt.password == "mqtt-password");     // the same host, another port
  CHECK(read("{\"mqtt\":{\"uri\":\"mqtt://other.lan\"}}", s).empty() && s.mqtt.password.empty());                        // another host: the password does not go along
  s.mqtt.password = "mqtt-password";
  CHECK(read("{\"mqtt\":{\"uri\":\"other.lan\",\"password_clear\":false}}", s).empty() && s.mqtt.password == "mqtt-password");   // no scheme: the host is read as it is
  CHECK(read("{\"server\":{\"host\":\"elsewhere.lan\"}}", s).empty() && s.server.password.empty());
  CHECK(read("{\"voice\":{\"url\":\"http://voice.lan:19000\"}}", s).empty() && s.voice.token == "voice-token");
  CHECK(read("{\"voice\":{\"url\":\"http://speech.lan:19000\"}}", s).empty() && s.voice.token.empty());
}

static void test_the_backup_lists() {
  config::Settings s = config::default_settings("a1b2c3");
  s.sta.backup.push_back({"casa", "casa-password"});
  CHECK(read("{\"sta\":{\"backup\":[{\"ssid\":\"casa\"},{\"ssid\":\"b\"},{\"ssid\":\"c\"},{\"ssid\":\"d\"}]}}", s).empty());
  CHECK(s.sta.backup.size() == config::kMaxBackupNetworks && s.sta.backup[0].password == "casa-password" && s.sta.backup[1].password.empty());
  CHECK(has(read("{\"sta\":{\"backup\":5}}", s), "sta.backup", "invalid"));
  CHECK(has(read("{\"sta\":{\"backup\":[7,{\"ssid\":\"x\",\"password\":\"abc\"}]}}", s), "sta.backup.0.ssid", "invalid"));
  CHECK(read("{\"sta\":{\"backup\":[]}}", s).empty() && s.sta.backup.empty());

  config::Settings m = config::default_settings("a1b2c3");
  m.mqtt.backup.push_back({"mqtt://one.lan", "user", "one-password"});
  m.mqtt.backup.push_back({"mqtt://two.lan", "user", "two-password"});
  CHECK(read("{\"mqtt\":{\"backup\":[{\"uri\":\"mqtt://one.lan\",\"username\":\"user\"},{\"uri\":\"mqtt://moved.lan\",\"username\":\"user\"},{\"uri\":\"mqtt://third.lan\"}]}}", m).empty());
  CHECK(m.mqtt.backup.size() == config::kMaxBackupBrokers && m.mqtt.backup[0].password == "one-password" && m.mqtt.backup[1].password.empty());
  CHECK(has(read("{\"mqtt\":{\"backup\":\"x\"}}", m), "mqtt.backup", "invalid"));
  CHECK(has(read("{\"mqtt\":{\"backup\":[3]}}", m), "mqtt.backup.0.uri", "invalid"));
  CHECK(has(read("{\"mqtt\":{\"backup\":[{\"uri\":5,\"username\":6}]}}", m), "mqtt.backup.0.uri", "invalid"));
}

static void test_the_choices_and_the_lists_of_a_document() {
  using config::AlarmFloor;
  config::Settings s = config::default_settings("a1b2c3");
  CHECK(read("{\"uplink\":\"ethernet\"}", s).empty() && s.uplink == config::Uplink::kEthernet);
  CHECK(has(read("{\"uplink\":\"carrier-pigeon\"}", s), "uplink", "invalid") && has(read("{\"uplink\":3}", s), "uplink", "invalid"));
  CHECK(read("{\"ap\":{\"security\":\"wpa3\"}}", s).empty() && s.ap.security == config::WifiSecurity::kWpa3);
  CHECK(has(read("{\"ap\":{\"security\":\"wep\"}}", s), "ap.security", "invalid"));
  CHECK(read("{\"audio\":{\"min_severity\":\"critical\"}}", s).empty() && s.audio.min_severity == AlarmFloor::kCritical);
  CHECK(has(read("{\"audio\":{\"min_severity\":\"loud\"}}", s), "audio.min_severity", "invalid"));
  CHECK(has(read("{\"audio\":{\"tone_by_source\":4}}", s), "audio.tone_by_source", "invalid"));
  CHECK(has(read("{\"audio\":{\"tone_by_source\":{\"" + std::string(hmi::source_name(0)) + "\":\"not-a-tone\"}}}", s), std::string("audio.tone_by_source.") + hmi::source_name(0), "invalid"));
  CHECK(has(read("{\"audio\":{\"tone_by_source\":{\"" + std::string(hmi::source_name(1)) + "\":7}}}", s), std::string("audio.tone_by_source.") + hmi::source_name(1), "invalid"));
  CHECK(read("{\"audio\":{\"tone_by_source\":{\"unknown_origin\":\"x\"}}}", s).empty());
  CHECK(has(read("{\"controls\":{\"ack\":\"never\"}}", s), "controls.ack", "invalid") && has(read("{\"controls\":{\"ack\":1}}", s), "controls.ack", "invalid"));
  CHECK(read("{\"controls\":{\"ack\":\"off\"}}", s).empty());
  CHECK(read("{\"web\":{\"mode\":\"https\"}}", s).empty() && s.web == config::WebMode::kHttps);
  CHECK(has(read("{\"web\":{\"mode\":\"ftp\"}}", s), "web.mode", "invalid"));
  CHECK(has(read("{\"web\":{\"admin_sources\":\"not a network list at all\"}}", s), "web.admin_sources", "invalid"));
  CHECK(has(read("{\"web\":{\"admin_sources\":" + std::string(1, '1') + "}}", s), "web.admin_sources", "invalid"));
  CHECK(read("{\"ble\":{\"mode\":\"always\"}}", s).empty() && s.ble == config::BleMode::kAlways);
  CHECK(read("{\"ble\":{\"mode\":\"off\"}}", s).empty() && s.ble == config::BleMode::kOff);
  CHECK(has(read("{\"ble\":{\"mode\":\"sometimes\"}}", s), "ble.mode", "invalid"));
  // the numbers that have a rule of their own
  CHECK(has(read("{\"audio\":{\"siren_repeat_s\":7}}", s), "audio.siren_repeat_s", "range") || s.audio.siren_repeat_s == 7);
  CHECK(has(read("{\"ap\":{\"bandwidth_mhz\":30}}", s), "ap.bandwidth_mhz", "range"));
  CHECK(read("{\"ap\":{\"bandwidth_mhz\":40}}", s).empty() && s.ap.bandwidth_mhz == 40);
  // the names of the enumerations
  CHECK(std::string(config::to_text(config::WifiSecurity::kOpen)) == "open" && std::string(config::to_text(config::WifiSecurity::kWpa2)) == "wpa2");
  CHECK(std::string(config::to_text(config::WifiSecurity::kWpa3)) == "wpa3" && std::string(config::to_text(config::WifiSecurity::kWpa2Wpa3)) == "wpa2wpa3");
  CHECK(std::string(config::to_text(config::Uplink::kEthernet)) == "ethernet" && std::string(config::to_text(config::Uplink::kWifi)) == "wifi");
  CHECK(std::string(config::to_text(config::BleMode::kOff)) == "off" && std::string(config::to_text(config::BleMode::kSetup)) == "setup" && std::string(config::to_text(config::BleMode::kAlways)) == "always");
  CHECK(std::string(config::to_text(config::WebMode::kHttp)) == "http" && std::string(config::to_text(config::WebMode::kBoth)) == "both" && std::string(config::to_text(config::WebMode::kHttps)) == "https");
}

// ---- the rules of the finished settings ---------------------------------------------------------------------------------------------

static config::Settings good() {
  config::Settings s = config::default_settings("a1b2c3");
  s.sta.enabled = true;
  s.sta.ssid = "casa";
  s.sta.password = "una-clave-larga";
  s.uplink = config::Uplink::kWifi;
  return s;
}

static void test_the_small_rules() {
  CHECK(config::time_zone_is_valid("CET-1CEST,M3.5.0,M10.5.0/3") && config::time_zone_is_valid("<+03>-3") && config::time_zone_is_valid("UTC0"));
  CHECK(!config::time_zone_is_valid("") && !config::time_zone_is_valid(std::string(49, 'A')) && !config::time_zone_is_valid("Europe Madrid") && !config::time_zone_is_valid("A;B"));
  for (const char* code : {"en", "es", "de", "fr", "it", "ja", "zh", "pt"}) CHECK(config::language_is_known(code));
  CHECK(!config::language_is_known("") && !config::language_is_known("xx") && !config::language_is_known("EN"));
  for (const int hours : {0, 1, 2, 3, 4, 6, 12, 24, 48}) CHECK(config::auto_restart_hours_is_valid(hours));
  CHECK(!config::auto_restart_hours_is_valid(5) && !config::auto_restart_hours_is_valid(-1) && !config::auto_restart_hours_is_valid(49));
  CHECK(config::valid_device_name("pump-1") && config::valid_device_name("a_b") && config::valid_device_name(std::string(32, 'a')));
  CHECK(!config::valid_device_name("") && !config::valid_device_name(std::string(33, 'a')) && !config::valid_device_name("-a") && !config::valid_device_name("_a") && !config::valid_device_name("A") && !config::valid_device_name("a b"));
  CHECK(config::broker_uri_is_valid("mqtt://broker.lan") && config::broker_uri_is_valid("mqtts://broker.lan:8883") && config::broker_uri_is_valid("mqtt://192.168.0.10:65535"));
  CHECK(!config::broker_uri_is_valid("http://broker.lan") && !config::broker_uri_is_valid("broker.lan") && !config::broker_uri_is_valid("mqtt://broker.lan:") && !config::broker_uri_is_valid("mqtt://broker.lan:123456"));
  CHECK(!config::broker_uri_is_valid("mqtt://broker.lan:80a") && !config::broker_uri_is_valid("mqtt://broker.lan:0") && !config::broker_uri_is_valid("mqtt://broker.lan:65536") && !config::broker_uri_is_valid("mqtt://bad host"));
  CHECK(config::service_url_is_valid("http://192.168.0.180:18090") && config::service_url_is_valid("https://voice.lan/") && config::service_url_is_valid("http://voice.lan"));
  CHECK(!config::service_url_is_valid("ftp://voice.lan") && !config::service_url_is_valid("voice.lan") && !config::service_url_is_valid("http://voice.lan/path") && !config::service_url_is_valid("http://user@voice.lan"));
  CHECK(!config::service_url_is_valid("http://voice.lan:") && !config::service_url_is_valid("http://voice.lan:99999x") && !config::service_url_is_valid("http://voice.lan:0") && !config::service_url_is_valid("http://voice.lan:70000"));
  config::AccessPoint ap;
  ap.channel = 5;
  CHECK(config::effective_channel(ap, 4) == 5);
  ap.channel = 0;
  CHECK(config::effective_channel(ap, 0) == 1 && config::effective_channel(ap, 1) == 6 && config::effective_channel(ap, 2) == 11 && config::effective_channel(ap, 5) == 11);
  ap.channel = 14;
  CHECK(config::effective_channel(ap, 1) == 6);
  config::Settings s = good();
  s.sta.enabled = false;
  s.uplink = config::Uplink::kWifi;
  CHECK(config::needs_own_network_after_setup(s) == !(board::kHasEthernet && s.uplink == config::Uplink::kEthernet));
  s.sta.enabled = true;
  s.sta.ssid = "casa";
  CHECK(!config::needs_own_network_after_setup(s));
  s.sta.ssid.clear();
  CHECK(config::needs_own_network_after_setup(s));
  s.ap.enabled = true;
  CHECK(!config::needs_own_network_after_setup(s));
}

static void test_validate_each_rule() {
  CHECK(config::validate(good()).empty());
  config::Settings s = good();
  s.node_id = "NO GOOD";
  s.node_name.clear();
  s.language = "xx";
  s.auto_restart_hours = 7;
  s.hostname = "-bad-";
  s.audio.critical_min_volume = 101;
  s.audio.siren_repeat_s = 5;
  s.audio.siren_max_repeats = 100000;
  s.audio.clip_max_bytes = 1;
  config::Problems p = config::validate(s);
  CHECK(has(p, "node.id", "invalid") && has(p, "node.name", "required") && has(p, "ui.language", "invalid") && has(p, "system.auto_restart_hours", "invalid") && has(p, "node.hostname", "invalid"));
  CHECK(has(p, "audio.critical_min_volume", "range") && has(p, "audio.siren_repeat_s", "range") && has(p, "audio.siren_max_repeats", "range") && has(p, "audio.clip_max_bytes", "range"));
  s = good();
  s.node_name = std::string(49, 'n');
  CHECK(has(config::validate(s), "node.name", "invalid"));
  s.node_name = "\xff\xfe";
  CHECK(has(config::validate(s), "node.name", "invalid"));
  s = good();
  s.audio.critical_min_volume = -1;
  s.audio.siren_max_repeats = 0;
  s.audio.clip_max_bytes = 2000000000;
  p = config::validate(s);
  CHECK(has(p, "audio.critical_min_volume", "range") && has(p, "audio.siren_max_repeats", "range") && has(p, "audio.clip_max_bytes", "range"));

  // the cable, with a fixed address
  s = good();
  s.uplink = config::Uplink::kEthernet;
  CHECK(board::kHasEthernet || has(config::validate(s), "uplink", "not_available"));
  s.ip.dhcp = false;
  s.ip.netmask.clear();
  p = config::validate(s);
  CHECK(has(p, "ip.address", "required") && has(p, "ip.netmask", "required") && has(p, "ip.gateway", "required"));
  s.ip.address = "999.1.1.1";
  s.ip.netmask = "255.0.255.0";
  s.ip.gateway = "abc";
  s.ip.dns1 = "abc";
  s.ip.dns2 = "def";
  p = config::validate(s);
  CHECK(has(p, "ip.address", "invalid") && has(p, "ip.netmask", "invalid") && has(p, "ip.gateway", "invalid") && has(p, "ip.dns1", "invalid") && has(p, "ip.dns2", "invalid"));
  s.ip.address = "192.168.1.0";
  s.ip.netmask = "255.255.255.0";
  s.ip.gateway = "192.168.1.1";
  s.ip.dns1.clear();
  s.ip.dns2.clear();
  CHECK(has(config::validate(s), "ip.address", "invalid"));                           // the network address itself
  s.ip.address = "192.168.1.255";
  CHECK(has(config::validate(s), "ip.address", "invalid"));                           // the broadcast address
  s.ip.address = "192.168.1.10";
  s.ip.gateway = "10.0.0.1";
  CHECK(has(config::validate(s), "ip.gateway", "outside_subnet"));
  s.ip.gateway = "192.168.1.10";
  CHECK(has(config::validate(s), "ip.gateway", "conflict"));
  s.ip.gateway = "192.168.1.1";
  s.ip.dns1 = "192.168.1.1";
  CHECK(!has(config::validate(s), "ip.address", "invalid") && !has(config::validate(s), "ip.gateway", "invalid") && !has(config::validate(s), "ip.dns1", "invalid"));

  // Wi-Fi: the access point, the network and its backups
  s = good();
  s.ap.enabled = true;
  s.ap.ssid.clear();
  s.ap.password.clear();
  p = config::validate(s);
  CHECK(has(p, "ap.ssid", "required") && has(p, "ap.password", "required"));
  s.ap.ssid = std::string(40, 's');
  s.ap.password = "short";
  p = config::validate(s);
  CHECK(has(p, "ap.ssid", "invalid") && has(p, "ap.password", "invalid_key"));
  s.ap.security = config::WifiSecurity::kOpen;
  CHECK(!has(config::validate(s), "ap.password", "invalid_key") && !has(config::validate(s), "ap.password", "required"));
  s = good();
  s.ap.country = "es";
  CHECK(has(config::validate(s), "ap.country", "invalid"));
  s.ap.country = "ESP";
  CHECK(has(config::validate(s), "ap.country", "invalid"));
  s = good();
  s.sta.ssid.clear();
  s.sta.password = "short";
  s.sta.backup.push_back({"", ""});
  s.sta.backup.push_back({std::string(40, 'x'), "short"});
  p = config::validate(s);
  CHECK(has(p, "sta.ssid", "required") && has(p, "sta.password", "invalid_key") && has(p, "sta.backup.0.ssid", "required") && has(p, "sta.backup.1.ssid", "invalid") && has(p, "sta.backup.1.password", "invalid_key"));
  s = good();
  s.sta.enabled = false;
  CHECK(has(config::validate(s), "sta.enabled", "required"));

  // the clock
  s = good();
  s.time.ntp = "bad host name";
  s.time.zone = "Europe Madrid";
  p = config::validate(s);
  CHECK(has(p, "time.ntp", "invalid") && has(p, "time.zone", "invalid"));
  s.time.ntp_enabled = false;
  CHECK(!has(config::validate(s), "time.ntp", "invalid"));

  // the broker
  s = good();
  s.mqtt.enabled = true;
  CHECK(has(config::validate(s), "mqtt.uri", "required"));
  s.mqtt.uri = "http://nope";
  CHECK(has(config::validate(s), "mqtt.uri", "invalid"));
  s.mqtt.uri = "mqtt://broker.lan";
  s.mqtt.backup.push_back({"", "", ""});
  s.mqtt.backup.push_back({"ftp://x", "", ""});
  p = config::validate(s);
  CHECK(has(p, "mqtt.backup.0.uri", "required") && has(p, "mqtt.backup.1.uri", "invalid"));

  // the server
  s = good();
  s.server.enabled = true;
  p = config::validate(s);
  CHECK(has(p, "server.host", "required") && has(p, "server.user", "required") && has(p, "server.password", "required"));
  s.server.host = "bad host";
  CHECK(has(config::validate(s), "server.host", "invalid"));
  s = good();
  s.server.cert_sha256 = "not a fingerprint";
  p = config::validate(s);
  CHECK(has(p, "server.cert_sha256", "invalid") && has(p, "server.cert_sha256", "needs_tls"));
  s.server.cert_sha256.clear();
  s.server.cert_pem = "not a certificate";
  p = config::validate(s);
  CHECK(has(p, "server.cert_pem", "invalid") && has(p, "server.cert_pem", "needs_tls"));
  s.server.tls = true;
  s.server.cert_pem = "-----BEGIN CERTIFICATE-----\nAAAA\n-----END CERTIFICATE-----\n";
  CHECK(!has(config::validate(s), "server.cert_pem", "invalid") && !has(config::validate(s), "server.cert_pem", "needs_tls"));
  s.server.cert_pem = "-----BEGIN CERTIFICATE-----\nAAAA\n";
  CHECK(has(config::validate(s), "server.cert_pem", "invalid"));

  // the screen and the voice
  s = good();
  s.display.night_from = 3;
  s.display.night_to = 3;
  s.display.night_brightness = 10;
  s.display.brightness = 80;
  CHECK(has(config::validate(s), "display.night_to", "conflict"));
  s.display.night_brightness = 80;
  CHECK(!has(config::validate(s), "display.night_to", "conflict"));
  s = good();
  s.voice.end_of_speech.silence_ms = 1;
  p = config::validate(s);
  bool voice_range = false;
  for (const config::Problem& problem : p) if (problem.path.rfind("voice.", 0) == 0 && problem.code == "range") voice_range = true;
  CHECK(voice_range);
  s = good();
  s.voice.enabled = true;
  s.voice.url.clear();
  s.voice.wake_name.clear();
  p = config::validate(s);
  CHECK(has(p, "voice.url", "required") && has(p, "voice.wake_name", "required"));
  s.voice.url = "ftp://x";
  s.voice.wake_name = std::string(25, 'w');
  p = config::validate(s);
  CHECK(has(p, "voice.url", "invalid") && has(p, "voice.wake_name", "invalid"));
}

static void test_load_and_the_two_forms_of_the_document() {
  config::Settings base = good();
  config::Settings out;
  config::Problems problems;
  CHECK(!config::load("not json", base, out, problems) && has(problems, "", "not_json"));
  problems.clear();
  CHECK(!config::load("{\"ip\":{\"dhcp\":\"maybe\"}}", base, out, problems) && has(problems, "ip.dhcp", "invalid"));   // a wrong member stops before the rules of the whole
  problems.clear();
  CHECK(!config::load("{\"ui\":{\"language\":\"xx\"}}", base, out, problems) && has(problems, "ui.language", "invalid"));
  problems.clear();
  CHECK(config::load("{\"ui\":{\"language\":\"es\"}}", base, out, problems) && out.language == "es");

  config::Settings s = good();
  s.sta.backup.push_back({"extra", "extra-password"});
  s.mqtt.backup.push_back({"mqtt://one.lan", "user", "one-password"});
  s.mqtt.password = "mqtt-password";
  const std::string with = config::to_json(s, true);
  const std::string without = config::to_json(s, false);
  CHECK(with.find("una-clave-larga") != std::string::npos && with.find("one-password") != std::string::npos && with.find("extra-password") != std::string::npos);
  CHECK(without.find("una-clave-larga") == std::string::npos && without.find("one-password") == std::string::npos && without.find("extra-password") == std::string::npos && without.find("mqtt-password") == std::string::npos);
  CHECK(without.find("\"password_set\":true") != std::string::npos);
  config::Settings again;
  problems.clear();
  CHECK(config::load(with, config::default_settings("a1b2c3"), again, problems) && again.sta.backup.size() == 1 && again.mqtt.backup.size() == 1 && again.mqtt.password == "mqtt-password");
  for (const config::AlarmFloor floor : {config::AlarmFloor::kWarning, config::AlarmFloor::kHigh, config::AlarmFloor::kCritical}) {
    s.audio.min_severity = floor;
    config::Settings round;
    problems.clear();
    CHECK(config::load(config::to_json(s, true), config::default_settings("a1b2c3"), round, problems) && round.audio.min_severity == floor);
  }
}

int main() {
  test_each_member_refuses_a_wrong_value();
  test_secrets_can_be_cleared_and_follow_their_host();
  test_the_backup_lists();
  test_the_choices_and_the_lists_of_a_document();
  test_the_small_rules();
  test_validate_each_rule();
  test_load_and_the_two_forms_of_the_document();
  FINISH("config edges");
}
