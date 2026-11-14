// ARMOR-NODE-HMI host tests - the broker link of the panel when no broker is set up (main/mqtt_link.cpp, unchanged): it makes no client, queues nothing and says so. Evidence: host double; nothing has run on a board.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include "check.hpp"
#include "mqtt_link.hpp"
#include "mqtt_link_double.hpp"

using namespace armor;

int main() {
  dbl::set_has_ip(true);
  for (int round = 0; round < 2; ++round) {
    config::Settings settings;
    settings.node_id = "hmi-1";
    if (round == 1) {   // enabled but with no address: the same
      settings.mqtt.enabled = true;
      settings.mqtt.uri = "";
    }
    mqtt_link::start(settings);
    mqtt_link::publish("armor/health/hmi-1", "nobody listens");
    mqtt_link::publish_retained("armor/control/hmi-1/maintenance", "active");
    CHECK(!dbl::wait_until([] { return dbl::client_count() > 0; }, 100));
    CHECK(!mqtt_link::connected());
    const mqtt_link::Status status = mqtt_link::status();
    CHECK(!status.enabled && !status.connected && status.published == 0 && status.dropped == 0);
  }
  FINISH("test_mqtt_link_off");
}
