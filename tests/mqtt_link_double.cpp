// ARMOR-NODE-HMI host tests - the double of esp-mqtt, FreeRTOS and the rest of ESP-IDF under main/mqtt_link.cpp (see mqtt_link_double.hpp). Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include "mqtt_link_double.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "network.hpp"
#include "node_guard.hpp"
#include "node_health.hpp"
#include "psram_mem.hpp"

// ---- FreeRTOS: tasks are threads, a tick is a microsecond --------------------------------------------------------------------------------------------------------------------

namespace {
struct TaskExit {};
}  // namespace

struct host_semaphore {
  std::mutex lock;
  std::condition_variable changed;
  int count = 0;
};

SemaphoreHandle_t xSemaphoreCreateBinary() { return new host_semaphore(); }

int xSemaphoreTake(SemaphoreHandle_t semaphore, TickType_t ticks) {
  std::unique_lock<std::mutex> guard(semaphore->lock);
  if (!semaphore->changed.wait_for(guard, std::chrono::microseconds(10 * static_cast<std::int64_t>(ticks)), [&] { return semaphore->count > 0; })) return pdFALSE;
  --semaphore->count;
  return pdTRUE;
}

int xSemaphoreGive(SemaphoreHandle_t semaphore) {
  {
    std::lock_guard<std::mutex> guard(semaphore->lock);
    semaphore->count = 1;
  }
  semaphore->changed.notify_all();
  return pdTRUE;
}

int xTaskCreate(TaskFunction_t function, const char*, std::uint32_t, void* argument, UBaseType_t, TaskHandle_t*) {
  std::thread([function, argument] {
    try {
      function(argument);
    } catch (const TaskExit&) {
      // vTaskDelete(nullptr): the task is over
    }
  }).detach();
  return pdTRUE;
}

void vTaskDelay(TickType_t ticks) { std::this_thread::sleep_for(std::chrono::microseconds(10 * static_cast<std::int64_t>(ticks))); }

void vTaskDelete(TaskHandle_t) { throw TaskExit(); }

// ---- the timer of the retries ------------------------------------------------------------------------------------------------------------------------------------------

struct host_timer {
  esp_timer_cb_t callback = nullptr;
  void* argument = nullptr;
};

namespace {
std::mutex g_timer_lock;
host_timer* g_timer = nullptr;
bool g_timer_started = false;
std::uint64_t g_timer_wait_us = 0;
}  // namespace

esp_err_t esp_timer_create(const esp_timer_create_args_t* args, esp_timer_handle_t* out) {
  auto* timer = new host_timer();
  timer->callback = args->callback;
  timer->argument = args->arg;
  {
    std::lock_guard<std::mutex> guard(g_timer_lock);
    g_timer = timer;
  }
  *out = timer;
  return ESP_OK;
}

esp_err_t esp_timer_start_once(esp_timer_handle_t, std::uint64_t microseconds) {
  std::lock_guard<std::mutex> guard(g_timer_lock);
  g_timer_started = true;
  g_timer_wait_us = microseconds;
  return ESP_OK;
}

std::int64_t esp_timer_get_time(void) {
  return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

// ---- the modules of the node that mqtt_link.cpp talks to ------------------------------------------------------------------------------------------------------------------

namespace {
std::atomic<bool> g_has_ip{false};
std::atomic<int> g_connected_notes{0}, g_disconnected_notes{0}, g_publish_notes{0}, g_health_notes{0};
}  // namespace

namespace armor::network {
bool has_ip() { return g_has_ip; }
}  // namespace armor::network

namespace armor::nodeguard {
void expect_task(const char*) {}
void note_broker_in_use() {}
void note_broker_connected(bool connected) { ++(connected ? g_connected_notes : g_disconnected_notes); }
void note_publish() { ++g_publish_notes; }
}  // namespace armor::nodeguard

namespace armor::node_health {
void note_broker_connected() { ++g_health_notes; }
}  // namespace armor::node_health

namespace armor::memory {
bool start_task(TaskFunction_t function, const char* name, std::uint32_t stack_bytes, void* argument, UBaseType_t priority, TaskHandle_t* handle, bool) {
  return xTaskCreate(function, name, stack_bytes, argument, priority, handle) == pdTRUE;
}
}  // namespace armor::memory

// ---- esp-mqtt ------------------------------------------------------------------------------------------------------------------------------------------------------------

struct host_mqtt_client {
  dbl::ClientView view;
  esp_event_handler_t handler = nullptr;
  void* argument = nullptr;
};

namespace {
std::mutex g_clients_lock;
std::vector<std::unique_ptr<host_mqtt_client>> g_clients;   // never freed during a run: a pointer the code still holds stays valid

std::string text(const char* value) { return value == nullptr ? std::string() : std::string(value); }

host_mqtt_client* at(std::size_t index) {
  std::lock_guard<std::mutex> guard(g_clients_lock);
  return index < g_clients.size() ? g_clients[index].get() : nullptr;
}

void dispatch(std::size_t index, esp_mqtt_event_t event) {
  host_mqtt_client* target = at(index);
  if (target == nullptr || target->handler == nullptr) return;
  event.client = target;
  target->handler(target->argument, "MQTT_EVENTS", event.event_id, &event);   // not under any lock: the handler calls back into this double
}
}  // namespace

esp_mqtt_client_handle_t esp_mqtt_client_init(const esp_mqtt_client_config_t* config) {
  auto client = std::make_unique<host_mqtt_client>();
  client->view.uri = text(config->broker.address.uri);
  client->view.client_id = text(config->credentials.client_id);
  client->view.username = text(config->credentials.username);
  client->view.password = text(config->credentials.authentication.password);
  client->view.keepalive = config->session.keepalive;
  client->view.will_topic = text(config->session.last_will.topic);
  client->view.will_msg = config->session.last_will.msg == nullptr ? std::string() : std::string(config->session.last_will.msg, static_cast<std::size_t>(config->session.last_will.msg_len));
  client->view.will_qos = config->session.last_will.qos;
  client->view.will_retain = config->session.last_will.retain;
  client->view.disable_auto_reconnect = config->network.disable_auto_reconnect;
  std::lock_guard<std::mutex> guard(g_clients_lock);
  g_clients.push_back(std::move(client));
  return g_clients.back().get();
}

esp_err_t esp_mqtt_client_register_event(esp_mqtt_client_handle_t client, esp_mqtt_event_id_t, esp_event_handler_t handler, void* argument) {
  client->handler = handler;
  client->argument = argument;
  return ESP_OK;
}

esp_err_t esp_mqtt_client_start(esp_mqtt_client_handle_t client) {
  std::lock_guard<std::mutex> guard(g_clients_lock);
  client->view.started = true;
  return ESP_OK;
}

esp_err_t esp_mqtt_client_stop(esp_mqtt_client_handle_t client) {
  std::lock_guard<std::mutex> guard(g_clients_lock);
  client->view.stopped = true;
  return ESP_OK;
}

esp_err_t esp_mqtt_client_destroy(esp_mqtt_client_handle_t client) {
  std::lock_guard<std::mutex> guard(g_clients_lock);
  client->view.destroyed = true;
  return ESP_OK;
}

esp_err_t esp_mqtt_client_reconnect(esp_mqtt_client_handle_t client) {
  std::lock_guard<std::mutex> guard(g_clients_lock);
  ++client->view.reconnects;
  return ESP_OK;
}

int esp_mqtt_client_publish(esp_mqtt_client_handle_t client, const char* topic, const char* data, int len, int qos, int retain) {
  std::lock_guard<std::mutex> guard(g_clients_lock);
  if (client->view.destroyed) return -1;
  dbl::Published message;
  message.topic = text(topic);
  message.data = data == nullptr ? std::string() : std::string(data, static_cast<std::size_t>(len));
  message.qos = qos;
  message.retain = retain;
  client->view.published.push_back(std::move(message));
  return 1;
}

int esp_mqtt_client_subscribe(esp_mqtt_client_handle_t client, const char* topic, int qos) {
  std::lock_guard<std::mutex> guard(g_clients_lock);
  client->view.subscribed.emplace_back(text(topic), qos);
  return 1;
}

// ---- what the tests use --------------------------------------------------------------------------------------------------------------------------------------------------

namespace dbl {

std::size_t client_count() {
  std::lock_guard<std::mutex> guard(g_clients_lock);
  return g_clients.size();
}

ClientView client(std::size_t index) {
  std::lock_guard<std::mutex> guard(g_clients_lock);
  return index < g_clients.size() ? g_clients[index]->view : ClientView();
}

void emit_connected(std::size_t index) {
  esp_mqtt_event_t event{};
  event.event_id = MQTT_EVENT_CONNECTED;
  dispatch(index, event);
}

void emit_disconnected(std::size_t index) {
  esp_mqtt_event_t event{};
  event.event_id = MQTT_EVENT_DISCONNECTED;
  dispatch(index, event);
}

void emit_error(std::size_t index) {
  esp_mqtt_event_t event{};
  event.event_id = MQTT_EVENT_ERROR;
  dispatch(index, event);
}

void emit_data(std::size_t index, const std::string& topic, const std::string& payload, int offset, int total) {
  std::string t = topic, p = payload;
  esp_mqtt_event_t event{};
  event.event_id = MQTT_EVENT_DATA;
  event.topic = t.data();
  event.topic_len = static_cast<int>(t.size());
  event.data = p.data();
  event.data_len = static_cast<int>(p.size());
  event.current_data_offset = offset;
  event.total_data_len = total < 0 ? static_cast<int>(p.size()) : total;
  dispatch(index, event);
}

void set_has_ip(bool has_ip) { g_has_ip = has_ip; }

bool retry_timer_started() {
  std::lock_guard<std::mutex> guard(g_timer_lock);
  const bool started = g_timer_started;
  g_timer_started = false;
  return started;
}

std::uint64_t retry_timer_wait_us() {
  std::lock_guard<std::mutex> guard(g_timer_lock);
  return g_timer_wait_us;
}

void fire_retry_timer() {
  host_timer* timer = nullptr;
  {
    std::lock_guard<std::mutex> guard(g_timer_lock);
    timer = g_timer;
  }
  if (timer != nullptr && timer->callback != nullptr) timer->callback(timer->argument);
}

int broker_connected_notes() { return g_connected_notes; }
int broker_disconnected_notes() { return g_disconnected_notes; }
int publish_notes() { return g_publish_notes; }
int health_connected_notes() { return g_health_notes; }

bool wait_until(const std::function<bool()>& condition, int timeout_ms) {
  for (int waited = 0; waited < timeout_ms; ++waited) {
    if (condition()) return true;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  return condition();
}

}  // namespace dbl
