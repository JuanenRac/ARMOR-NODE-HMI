// ARMOR-NODE-HMI host double of ESP-IDF - mqtt_client.h: the types and the calls of esp-mqtt that main/mqtt_link.cpp uses (the double itself is in mqtt_link_double.cpp). Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <cstddef>
#include <cstdint>

#include "esp_err.h"

typedef const char* esp_event_base_t;
struct host_mqtt_client;
typedef host_mqtt_client* esp_mqtt_client_handle_t;

enum esp_mqtt_event_id_t { MQTT_EVENT_ANY = -1, MQTT_EVENT_ERROR = 0, MQTT_EVENT_CONNECTED, MQTT_EVENT_DISCONNECTED, MQTT_EVENT_SUBSCRIBED, MQTT_EVENT_UNSUBSCRIBED, MQTT_EVENT_PUBLISHED, MQTT_EVENT_DATA };

struct esp_mqtt_event_t {
  esp_mqtt_event_id_t event_id;
  esp_mqtt_client_handle_t client;
  char* data;
  int data_len;
  char* topic;
  int topic_len;
  int current_data_offset;
  int total_data_len;
};
typedef esp_mqtt_event_t* esp_mqtt_event_handle_t;

struct esp_mqtt_client_config_t {
  struct { struct { const char* uri = nullptr; } address; struct { const char* certificate = nullptr; std::size_t certificate_len = 0; bool use_global_ca_store = false; bool skip_cert_common_name_check = false; } verification; } broker;
  struct { const char* client_id = nullptr; const char* username = nullptr; struct { const char* password = nullptr; } authentication; } credentials;
  struct { int keepalive = 0; struct { const char* topic = nullptr; const char* msg = nullptr; int msg_len = 0; int qos = 0; int retain = 0; } last_will; } session;
  struct { bool disable_auto_reconnect = false; } network;
  struct { int stack_size = 0; } task;
};

typedef void (*esp_event_handler_t)(void* handler_args, esp_event_base_t base, std::int32_t event_id, void* event_data);

esp_mqtt_client_handle_t esp_mqtt_client_init(const esp_mqtt_client_config_t* config);
esp_err_t esp_mqtt_client_register_event(esp_mqtt_client_handle_t client, esp_mqtt_event_id_t event, esp_event_handler_t handler, void* argument);
esp_err_t esp_mqtt_client_start(esp_mqtt_client_handle_t client);
esp_err_t esp_mqtt_client_stop(esp_mqtt_client_handle_t client);
esp_err_t esp_mqtt_client_destroy(esp_mqtt_client_handle_t client);
esp_err_t esp_mqtt_client_reconnect(esp_mqtt_client_handle_t client);
int esp_mqtt_client_publish(esp_mqtt_client_handle_t client, const char* topic, const char* data, int len, int qos, int retain);
int esp_mqtt_client_subscribe(esp_mqtt_client_handle_t client, const char* topic, int qos);
