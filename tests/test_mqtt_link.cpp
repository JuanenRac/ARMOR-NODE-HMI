// ARMOR-NODE-HMI host tests - the real broker link of the panel (main/mqtt_link.cpp, unchanged) against the double of esp-mqtt: the client it makes, what it subscribes to and publishes when the
// broker answers, the messages it hands over, the queue of what the node says, the retries that grow and the move to the next saved broker. Evidence: host double; nothing has run on a board.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <string>
#include <utility>
#include <vector>

#include "check.hpp"
#include "mqtt_link.hpp"
#include "mqtt_link_double.hpp"
#include "core/mqtt_topic.hpp"

using namespace armor;

namespace {
std::vector<std::pair<std::string, std::string>> g_control, g_all;
int g_control_connected = 0, g_all_connected = 0;

bool has(const dbl::ClientView& view, const std::string& topic, const std::string& data, int qos, int retain) {
  for (const dbl::Published& message : view.published) {
    if (message.topic == topic && message.data == data && message.qos == qos && message.retain == retain) return true;
  }
  return false;
}
}  // namespace

int main() {
  const std::string node = "hmi-1";
  const std::string control = "armor/control/hmi-1/command";

  mqtt_link::subscribe("armor/control/hmi-1/#", [](const std::string& topic, const std::string& payload) { g_control.emplace_back(topic, payload); }, [] { ++g_control_connected; });
  mqtt_link::subscribe("armor/control/all/command", [](const std::string& topic, const std::string& payload) { g_all.emplace_back(topic, payload); }, [] { ++g_all_connected; });
  mqtt_link::subscribe("", nullptr);   // an empty filter is not a subscription

  // nothing is sent before the link is set up
  mqtt_link::publish("armor/health/hmi-1", "early");
  CHECK(!mqtt_link::connected());
  CHECK(!mqtt_link::status().enabled);

  config::Settings settings;
  settings.node_id = node;
  settings.mqtt.enabled = true;
  settings.mqtt.uri = "mqtt://primary.local:1883";
  settings.mqtt.username = "panel";
  settings.mqtt.password = "primary-pass";
  settings.mqtt.heartbeat_s = 10;
  settings.mqtt.backup.push_back({"mqtt://backup-one.local:1883", "panel-b1", "b1-pass"});
  settings.mqtt.backup.push_back({"mqtt://backup-two.local:1883", "panel-b2", "b2-pass"});
  mqtt_link::start(settings);
  CHECK(mqtt_link::status().enabled);

  // without an address the link waits: no client is made
  CHECK(!dbl::wait_until([] { return dbl::client_count() > 0; }, 80));
  dbl::set_has_ip(true);
  CHECK(dbl::wait_until([] { return dbl::client_count() == 1; }));
  const dbl::ClientView first = dbl::client(0);
  CHECK(first.uri == "mqtt://primary.local:1883" && first.username == "panel" && first.password == "primary-pass" && first.client_id == node);
  CHECK(first.keepalive == 20);                                       // twice the heartbeat
  CHECK(first.will_topic == mqtt::will_topic(node) && first.will_msg == mqtt::will_payload(node) && first.will_qos == 1 && first.will_retain == 1);
  CHECK(first.disable_auto_reconnect && first.started);               // the retries are the node's, not the client's

  // the broker answers: the node says it is online (retained) and subscribes to what it was asked to listen to
  dbl::emit_connected(0);
  CHECK(mqtt_link::connected() && mqtt_link::status().connected);
  const dbl::ClientView online = dbl::client(0);
  CHECK(has(online, mqtt::will_topic(node), mqtt::birth_payload(node), 1, 1));
  CHECK(online.subscribed.size() == 2 && online.subscribed[0] == std::make_pair(std::string("armor/control/hmi-1/#"), 1) && online.subscribed[1] == std::make_pair(std::string("armor/control/all/command"), 1));
  CHECK(g_control_connected == 1 && g_all_connected == 1);
  CHECK(dbl::broker_connected_notes() == 1 && dbl::health_connected_notes() == 1);

  // what the broker sends goes to the subscriptions it matches, whole messages only
  dbl::emit_data(0, control, "{\"action\":\"identify\"}");
  CHECK(g_control.size() == 1 && g_control[0].first == control && g_control[0].second == "{\"action\":\"identify\"}" && g_all.empty());
  dbl::emit_data(0, "armor/control/all/command", "restart");
  CHECK(g_all.size() == 1 && g_all[0].second == "restart" && g_control.size() == 1);
  dbl::emit_data(0, control, "piece", 5, 20);       // a fragment: never acted on
  dbl::emit_data(0, control, "piece", 0, 20);       // the beginning of a longer message
  dbl::emit_data(0, "armor/other/x", "nobody listens");
  CHECK(g_control.size() == 1 && g_all.size() == 1);

  // what the node says goes through the queue, in order, and reaches the client with its quality of service
  mqtt_link::publish("armor/health/hmi-1", "report-1");
  mqtt_link::publish_retained("armor/control/hmi-1/maintenance", "active");
  mqtt_link::publish("armor/health/hmi-1", "report-2");
  CHECK(dbl::wait_until([] { return dbl::client(0).published.size() >= 4; }));
  const dbl::ClientView sent = dbl::client(0);
  CHECK(sent.published.size() == 4);   // the birth and the three
  CHECK(has(sent, "armor/health/hmi-1", "report-1", 0, 0) && has(sent, "armor/control/hmi-1/maintenance", "active", 1, 1) && has(sent, "armor/health/hmi-1", "report-2", 0, 0));
  CHECK(sent.published[1].data == "report-1" && sent.published[2].data == "active" && sent.published[3].data == "report-2");
  CHECK(mqtt_link::status().published == 3 && dbl::publish_notes() >= 3);

  // the broker is lost: nothing is queued while it is down, and the retry comes after a wait
  dbl::emit_disconnected(0);
  CHECK(!mqtt_link::connected() && dbl::broker_disconnected_notes() == 1);
  const std::uint32_t dropped_before = mqtt_link::status().dropped;
  mqtt_link::publish("armor/health/hmi-1", "while-down");
  CHECK(mqtt_link::status().dropped == dropped_before + 1);
  CHECK(dbl::retry_timer_started());
  CHECK(dbl::retry_timer_wait_us() >= 500000 && dbl::retry_timer_wait_us() <= 2000000);   // the first wait is about a second
  dbl::fire_retry_timer();
  CHECK(dbl::client(0).reconnects == 1);
  dbl::emit_error(0);                                // the broker refuses: another wait, longer than the first one can be
  CHECK(dbl::retry_timer_started());
  CHECK(dbl::client_count() == 1);                   // the same client is tried again, no second client yet

  // the broker stays down: after a few rounds of the watchdog the node moves to the next saved one, and the old client is stopped and freed
  CHECK(dbl::wait_until([] { return dbl::client_count() == 2; }, 5000));
  const dbl::ClientView old_one = dbl::client(0);
  CHECK(old_one.stopped && old_one.destroyed);
  const dbl::ClientView second = dbl::client(1);
  CHECK(second.uri == "mqtt://backup-one.local:1883" && second.username == "panel-b1" && second.password == "b1-pass" && second.client_id == node && second.started);
  CHECK(second.will_topic == mqtt::will_topic(node));

  // the saved broker answers: the same subscriptions are made on the new client and the node says it is online again
  dbl::emit_connected(1);
  CHECK(mqtt_link::connected());
  const dbl::ClientView back = dbl::client(1);
  CHECK(has(back, mqtt::will_topic(node), mqtt::birth_payload(node), 1, 1) && back.subscribed.size() == 2);
  CHECK(g_control_connected == 2 && g_all_connected == 2);
  mqtt_link::publish("armor/health/hmi-1", "on-the-backup");
  CHECK(dbl::wait_until([] { return has(dbl::client(1), "armor/health/hmi-1", "on-the-backup", 0, 0); }));
  CHECK(!has(dbl::client(0), "armor/health/hmi-1", "on-the-backup", 0, 0));   // the old client is not used any more

  // a wall clock and a time since 1970 are what the callers use for their stamps
  CHECK(mqtt_link::wall_clock_ms() > 1700000000000ULL);

  FINISH("test_mqtt_link");
}
