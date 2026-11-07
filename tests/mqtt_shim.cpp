// ARMOR-NODE-ELECTRICAL - tests/mqtt_shim.cpp: the real decoding and message code of the node as a program for a computer, for the MQTT shim of ARMOR-SIMULATOR.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
//   mqtt_shim <node-id>      standard input, one round of the meters per line:  timestamp_ms <TAB> channel:model:address:reply-hex | channel:model:address:reply-hex ...
//                            standard output, one message per line:              topic <TAB> payload
//
// It is built from the core files of the firmware (the Modbus decoder of the PZEM meters and the writer of the electrical message) and does not talk to any broker: the
// simulator publishes what it prints, so the server receives exactly the bytes the node would have sent. A reply that does not decode is left out of the message, as the node
// leaves out a meter with no fresh reading. Nothing here has run on a board.
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "../core/electrical_json.hpp"
#include "../core/mqtt_topic.hpp"
#include "../core/pzem.hpp"
#include "../core/pzem_bus.hpp"

namespace {
std::vector<std::string> split(const std::string& text, char sep) {
  std::vector<std::string> out;
  std::string item;
  for (char c : text) { if (c == sep) { out.push_back(item); item.clear(); } else item += c; }
  out.push_back(item);
  return out;
}
std::vector<std::uint8_t> unhex(const std::string& hex) {
  std::vector<std::uint8_t> out;
  for (std::size_t i = 0; i + 1 < hex.size(); i += 2) out.push_back(static_cast<std::uint8_t>(std::stoi(hex.substr(i, 2), nullptr, 16)));
  return out;
}
}  // namespace

int main(int argc, char** argv) {
  namespace pz = armor::electrical::pzem;
  const std::string node = argc > 1 ? argv[1] : "";
  const std::string health_topic = armor::mqtt::will_topic(node);
  if (!health_topic.empty()) std::cout << health_topic << '\t' << armor::mqtt::birth_payload(node) << '\n';
  std::string line;
  while (std::getline(std::cin, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    const std::size_t tab = line.find('\t');
    if (tab == std::string::npos) continue;
    const std::uint64_t now_ms = std::stoull(line.substr(0, tab));
    std::vector<armor::electrical::MeterState> states;
    for (const std::string& item : split(line.substr(tab + 1), '|')) {
      const auto f = split(item, ':');
      if (f.size() != 4) continue;
      armor::electrical::MeterState state;
      state.config.channel = f[0];
      state.config.model = f[1] == "pzem-004t" ? pz::Model::kAc : pz::Model::kDc;
      state.config.address = static_cast<std::uint8_t>(std::stoi(f[2]));
      const auto reply = unhex(f[3]);
      if (pz::decode(state.config.model, state.config.address, reply.data(), reply.size(), state.reading) != pz::Result::kOk) continue;
      state.have_reading = true;
      state.reading_at_ms = now_ms;
      states.push_back(state);
    }
    std::vector<const armor::electrical::MeterState*> pointers;
    for (const auto& state : states) pointers.push_back(&state);
    const std::string topic = armor::electrical::topic(node);
    if (topic.empty()) continue;
    std::cout << topic << '\t' << armor::electrical::message_json(node, now_ms, false, pointers, {}, "", {}, now_ms) << '\n';
  }
  return 0;
}
