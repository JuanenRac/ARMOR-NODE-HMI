// ARMOR-NODE-HMI - the panel's own record of events in the flash partition "storage" (see event_store.hpp). The rules are in core/event_log.hpp; this file is the files of the partition.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include "event_store.hpp"

#include <cstdio>
#include <ctime>
#include <memory>
#include <mutex>

#include "esp_log.h"
#include "esp_spiffs.h"
#include "core/event_log.hpp"

namespace armor::event_store {
namespace {
constexpr char kTag[] = "armor-events";
constexpr char kBase[] = "/storage";
constexpr char kLabel[] = "storage";

std::mutex g_lock;
bool g_active = false;

// The files of the partition. SPIFFS has no folders: every name is a file of the base path.
class SpiffsFiles final : public hmi::LogFiles {
 public:
  bool append(const std::string& name, std::string_view data) override {
    FILE* file = std::fopen(path(name).c_str(), "ab");
    if (file == nullptr) return false;
    const bool ok = std::fwrite(data.data(), 1, data.size(), file) == data.size();
    return std::fclose(file) == 0 && ok;
  }
  long size(const std::string& name) override {
    FILE* file = std::fopen(path(name).c_str(), "rb");
    if (file == nullptr) return -1;
    std::fseek(file, 0, SEEK_END);
    const long length = std::ftell(file);
    std::fclose(file);
    return length;
  }
  bool rename(const std::string& from, const std::string& to) override {
    std::remove(path(to).c_str());   // a rename onto an existing file fails on SPIFFS
    return std::rename(path(from).c_str(), path(to).c_str()) == 0;
  }
  bool remove(const std::string& name) override { return std::remove(path(name).c_str()) == 0; }
  long free_bytes() override {
    std::size_t total = 0, used = 0;
    if (esp_spiffs_info(kLabel, &total, &used) != ESP_OK || used > total) return -1;
    return static_cast<long>(total - used);
  }
  bool read(const std::string& name, std::string& out, std::size_t max_bytes) override {
    FILE* file = std::fopen(path(name).c_str(), "rb");
    if (file == nullptr) return false;
    std::fseek(file, 0, SEEK_END);
    const long length = std::ftell(file);
    const long from = length > static_cast<long>(max_bytes) ? length - static_cast<long>(max_bytes) : 0;
    std::fseek(file, from, SEEK_SET);
    out.assign(static_cast<std::size_t>(length - from), '\0');
    const std::size_t got = out.empty() ? 0 : std::fread(&out[0], 1, out.size(), file);
    std::fclose(file);
    out.resize(got);
    return true;
  }

 private:
  static std::string path(const std::string& name) { return std::string(kBase) + "/" + name; }
};

SpiffsFiles g_files;
std::unique_ptr<hmi::EventLog> g_log;
}  // namespace

bool start(const config::Settings& settings) {
  std::lock_guard<std::mutex> guard(g_lock);
  if (!settings.storage.enabled || g_active) return g_active;
  esp_vfs_spiffs_conf_t conf{};
  conf.base_path = kBase;
  conf.partition_label = kLabel;
  conf.max_files = 4;
  conf.format_if_mount_failed = true;   // the partition is the panel's own storage and starts empty: the first mount formats it
  if (esp_vfs_spiffs_register(&conf) != ESP_OK) { ESP_LOGW(kTag, "the storage partition could not be mounted: no record of events"); return false; }
  g_log = std::make_unique<hmi::EventLog>(g_files);
  g_log->configure(settings.storage.max_kb, settings.storage.keep);
  g_active = true;
  ESP_LOGI(kTag, "the record of events is on (%d KiB a file, %d files)", settings.storage.max_kb, settings.storage.keep);
  return true;
}

bool active() {
  std::lock_guard<std::mutex> guard(g_lock);
  return g_active;
}

void add(std::string_view code, std::string_view detail) {
  std::lock_guard<std::mutex> guard(g_lock);
  if (!g_active || !g_log) return;
  g_log->add(static_cast<std::int64_t>(std::time(nullptr)), code, detail);
}

std::string text(std::size_t max_bytes) {
  std::lock_guard<std::mutex> guard(g_lock);
  if (!g_active || !g_log) return std::string();
  return g_log->text(max_bytes);
}

}  // namespace armor::event_store
