// ARMOR-NODE-HMI - the panel's own record of what happened to it (mains lost, battery low, ...), kept in a few files that rotate, so that it never fills the storage. Pure: the files are reached
// through a small interface, so all of it is tested on a computer with an in-memory stand-in. Nothing of it has run on a board.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// Off by default (setting storage.enabled). One line per event: "<unix seconds> <code> <detail>". The current file is events.log; when the next line would take it past max_kb it becomes
// events.1, events.1 becomes events.2, and so on up to `keep` files (the oldest is deleted): the record never takes more than max_kb times keep. A line is cut to 160 characters, a code is
// letters, digits and "_-." only, and a detail is printable ASCII (anything else becomes "?", a newline included), so one event is always one line. When the storage has less than
// reserve_kb free the event is not written (and counted), so the record never leaves the storage full for the other things that live there.
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace armor::hmi {

class LogFiles {
 public:
  virtual ~LogFiles() = default;
  virtual bool append(const std::string& name, std::string_view data) = 0;
  virtual long size(const std::string& name) = 0;                     // -1 when the file does not exist
  virtual bool rename(const std::string& from, const std::string& to) = 0;
  virtual bool remove(const std::string& name) = 0;
  virtual long free_bytes() = 0;
  virtual bool read(const std::string& name, std::string& out, std::size_t max_bytes) = 0;
};

constexpr std::size_t kLogLineMax = 160;
constexpr int kLogMinKb = 16, kLogMaxKb = 4096, kLogDefaultKb = 256;
constexpr int kLogMinKeep = 2, kLogMaxKeep = 10, kLogDefaultKeep = 4;

struct StorageSettings {
  bool enabled = false;
  int max_kb = kLogDefaultKb;
  int keep = kLogDefaultKeep;
};

class EventLog {
 public:
  explicit EventLog(LogFiles& files) : files_(files) {}

  void configure(int max_kb, int keep, int reserve_kb = 64) {
    max_bytes_ = static_cast<long>(max_kb < kLogMinKb ? kLogMinKb : (max_kb > kLogMaxKb ? kLogMaxKb : max_kb)) * 1024;
    keep_ = keep < kLogMinKeep ? kLogMinKeep : (keep > kLogMaxKeep ? kLogMaxKeep : keep);
    reserve_bytes_ = static_cast<long>(reserve_kb < 0 ? 0 : reserve_kb) * 1024;
  }

  // Writes one event. False when it could not be (the storage is nearly full, or a write failed); the counters say how often.
  bool add(std::int64_t unix_seconds, std::string_view code, std::string_view detail) {
    const std::string line = make_line(unix_seconds, code, detail);
    if (files_.free_bytes() >= 0 && files_.free_bytes() < reserve_bytes_ + static_cast<long>(line.size())) { ++dropped_; return false; }
    const long current = files_.size("events.log");
    if (current >= 0 && current + static_cast<long>(line.size()) > max_bytes_ && current > 0) rotate();
    if (!files_.append("events.log", line)) { ++failed_; return false; }
    ++written_;
    return true;
  }

  // The record as text, oldest line first, at most max_bytes of the newest ones.
  std::string text(std::size_t max_bytes) {
    std::string all;
    for (int index = keep_ - 1; index >= 1; --index) {
      std::string part;
      if (files_.read(name_of(index), part, max_bytes)) all += part;
    }
    std::string part;
    if (files_.read("events.log", part, max_bytes)) all += part;
    if (all.size() > max_bytes) {
      all.erase(0, all.size() - max_bytes);
      const std::size_t cut = all.find('\n');   // start at a whole line
      if (cut != std::string::npos) all.erase(0, cut + 1);
    }
    return all;
  }

  std::uint32_t written() const { return written_; }
  std::uint32_t dropped() const { return dropped_; }
  std::uint32_t failed() const { return failed_; }
  long max_bytes() const { return max_bytes_; }
  int keep() const { return keep_; }

  static std::string make_line(std::int64_t unix_seconds, std::string_view code, std::string_view detail) {
    std::string line = std::to_string(static_cast<long long>(unix_seconds < 0 ? 0 : unix_seconds)) + " ";
    std::string clean_code;
    for (char c : code) {
      const bool plain = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
      if (clean_code.size() < 32) clean_code += plain ? c : '_';
    }
    line += clean_code.empty() ? std::string("event") : clean_code;
    line += ' ';
    for (char c : detail) {
      if (line.size() >= kLogLineMax - 1) break;
      line += (static_cast<unsigned char>(c) >= 0x20 && static_cast<unsigned char>(c) < 0x7F) ? c : '?';
    }
    if (line.size() > kLogLineMax - 1) line.resize(kLogLineMax - 1);
    line += '\n';
    return line;
  }

 private:
  static std::string name_of(int index) { return "events." + std::to_string(index); }

  void rotate() {
    files_.remove(name_of(keep_ - 1));
    for (int index = keep_ - 2; index >= 1; --index) files_.rename(name_of(index), name_of(index + 1));
    files_.rename("events.log", name_of(1));
  }

  LogFiles& files_;
  long max_bytes_ = kLogDefaultKb * 1024L;
  int keep_ = kLogDefaultKeep;
  long reserve_bytes_ = 64 * 1024L;
  std::uint32_t written_ = 0, dropped_ = 0, failed_ = 0;
};

}  // namespace armor::hmi
