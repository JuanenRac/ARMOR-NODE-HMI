// ARMOR-NODE-HMI host tests - the double of esp-mqtt, FreeRTOS and the rest of ESP-IDF under main/mqtt_link.cpp: what the test uses to play the broker. Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// main/mqtt_link.cpp is compiled unchanged. The calls it makes to esp-mqtt are recorded here (the clients that were made, with the settings they were given, what they published and
// subscribed to) and the events a broker would cause are made by hand (emit). A task is a thread, a tick of FreeRTOS lasts ten microseconds, and the timer of the retries fires when the
// test says so. Nothing here opens a socket to a broker. Evidence: host double; nothing has run on a board.
#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "mqtt_client.h"

namespace dbl {

struct Published {
  std::string topic, data;
  int qos = 0, retain = 0;
};

// What the code asked of one client (a copy: the strings of the configuration live only as long as the caller keeps them).
struct ClientView {
  std::string uri, client_id, username, password, will_topic, will_msg;
  int keepalive = 0, will_qos = 0, will_retain = 0;
  bool disable_auto_reconnect = false;
  bool started = false, stopped = false, destroyed = false;
  int reconnects = 0;
  std::vector<Published> published;
  std::vector<std::pair<std::string, int>> subscribed;   // filter, qos
};

std::size_t client_count();
ClientView client(std::size_t index);

// A broker event for client `index`: connected, disconnected, error, or a message (a complete one unless `offset` or `total` say it is a piece).
void emit_connected(std::size_t index);
void emit_disconnected(std::size_t index);
void emit_error(std::size_t index);
void emit_data(std::size_t index, const std::string& topic, const std::string& payload, int offset = 0, int total = -1);

// The network: false keeps the link task waiting for an address.
void set_has_ip(bool has_ip);
// The timer of the retries: whether it was started (since the last call) and a way to fire it, as the timer of the chip does after its wait.
bool retry_timer_started();
std::uint64_t retry_timer_wait_us();
void fire_retry_timer();

// Counters of what the guard and the health report were told.
int broker_connected_notes();
int broker_disconnected_notes();
int publish_notes();
int health_connected_notes();

// Waits (up to `timeout_ms`) until `condition` is true; the tasks of the code run on threads of their own.
bool wait_until(const std::function<bool()>& condition, int timeout_ms = 3000);

}  // namespace dbl
