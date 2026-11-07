// ARMOR-NODE-HMI - tests/mqtt_shim.cpp: the real health-report code of the node as a program for a computer, for the MQTT shim of ARMOR-SIMULATOR.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
//   mqtt_shim <node-id>      standard input, one report per line, fields separated by a TAB:
//                              timestamp_ms <TAB> uptime_s <TAB> reset_code <TAB> heap_free <TAB> heap_min <TAB> link <TAB> rssi_dbm <TAB> broker_reconnects <TAB> last_error
//                              reset_code is the number esp_reset_reason() gives (1 power on, 9 brownout ...); link is wifi, ethernet or none; last_error may be empty
//                            standard output, one message per line:  topic <TAB> payload
//
// It is built from the core files of the firmware (the reset reasons, the link rules and the writer of the health report, the one every family of node sends) and does not talk
// to any broker: the simulator publishes what it prints, so the server receives exactly the bytes the node would have sent. The presence messages of the panel and the screen
// are not in it. Nothing here has run on a board.
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "../core/node_health.hpp"

namespace {
std::vector<std::string> split(const std::string& text, char sep) {
  std::vector<std::string> out;
  std::string item;
  for (char c : text) { if (c == sep) { out.push_back(item); item.clear(); } else item += c; }
  out.push_back(item);
  return out;
}
}  // namespace

int main(int argc, char** argv) {
  namespace nh = armor::nodehealth;
  const std::string node = argc > 1 ? argv[1] : "";
  const std::string topic = nh::topic(node);
  if (topic.empty()) return 0;
  std::string line;
  while (std::getline(std::cin, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    const std::vector<std::string> f = split(line, '\t');
    if (f.size() < 9) continue;
    nh::Report report;
    report.node_id = node;
    report.family = "hmi";
    report.firmware = "0.4.6";
    report.board = "shim-host";
    report.contract = "0.8.1";
    report.timestamp_ms = std::stoull(f[0]);
    report.uptime_s = std::stoull(f[1]);
    report.reset_reason = nh::reset_reason_text(std::stoi(f[2]));
    report.heap_free = std::stoull(f[3]);
    report.heap_min = std::stoull(f[4]);
    report.link.link = f[5] == "wifi" ? nh::Link::kWifi : f[5] == "ethernet" ? nh::Link::kEthernet : nh::Link::kNone;
    report.link.up = report.link.link != nh::Link::kNone;
    report.link.rssi_dbm = std::stoi(f[6]);
    report.broker_reconnects = static_cast<std::uint32_t>(std::stoul(f[7]));
    report.last_error = f[8];
    std::cout << topic << '\t' << nh::to_json(report) << '\n';
  }
  return 0;
}
