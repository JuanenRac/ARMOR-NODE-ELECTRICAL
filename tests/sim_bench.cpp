// ARMOR-NODE-ELECTRICAL - tests/sim_bench.cpp: the driver of the simulation bench. The Modbus-RTU replies of the PZEM meters go through the real decoder.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// It is built from the core files of the firmware, on a computer, and fed by ARMOR-SIMULATOR (python -m armor_simulator.node_bench) with the buses of the equipment, spoiled
// on purpose. Nothing here has run on a board.
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include "../core/pzem.hpp"

// The line protocol shared by the drivers of every node (see ARMOR-SIMULATOR docs/NODE_BENCH.md): one case per line on the standard input,
//   bus <TAB> id <TAB> arg <TAB> payload        (payload: hex of bytes or text; "-" when there is none)
// and one answer per case on the standard output: id <TAB> answer.
namespace simio {
inline std::vector<std::string> split(const std::string& text, char sep) {
  std::vector<std::string> out;
  std::string item;
  for (char c : text) { if (c == sep) { out.push_back(item); item.clear(); } else item += c; }
  out.push_back(item);
  return out;
}
inline std::vector<std::uint8_t> unhex(const std::string& hex) {
  std::vector<std::uint8_t> out;
  if (hex == "-") return out;
  for (std::size_t i = 0; i + 1 < hex.size(); i += 2) out.push_back(static_cast<std::uint8_t>(std::stoi(hex.substr(i, 2), nullptr, 16)));
  return out;
}
inline std::string unhex_text(const std::string& hex) { const auto bytes = unhex(hex); return std::string(bytes.begin(), bytes.end()); }
inline std::string fixed(double value, int digits) { char buffer[64]; std::snprintf(buffer, sizeof buffer, "%.*f", digits, value); return buffer; }
struct Case { std::string bus, id, arg, payload; };
inline bool next_case(Case& out) {
  std::string line;
  while (std::getline(std::cin, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    const auto f = split(line, '\t');
    if (f.size() != 4) continue;
    out = Case{f[0], f[1], f[2], f[3]};
    return true;
  }
  return false;
}
}  // namespace simio

static const char* const BUS = "pzem";

static const char* name(armor::electrical::pzem::Result result) {
  using R = armor::electrical::pzem::Result;
  switch (result) {
    case R::kOk: return "ok";
    case R::kTooShort: return "too_short";
    case R::kCrc: return "crc";
    case R::kAddress: return "address";
    case R::kException: return "exception";
    case R::kFunction: return "function";
    case R::kLength: return "length";
    case R::kRange: return "range";
  }
  return "?";
}

// The payload is the hex of a text: replies separated by '|', each "model:address:reply-hex"; the answer is one result per reply.
static std::string answer(const simio::Case& c) {
  namespace pz = armor::electrical::pzem;
  std::string text;
  bool first = true;
  for (const std::string& item : simio::split(simio::unhex_text(c.payload), '|')) {
    const auto f = simio::split(item, ':');
    if (f.size() != 3) continue;
    const pz::Model model = f[0] == "pzem-004t" ? pz::Model::kAc : pz::Model::kDc;
    const auto reply = simio::unhex(f[2]);
    pz::Reading reading;
    const pz::Result result = pz::decode(model, static_cast<std::uint8_t>(std::stoi(f[1])), reply.data(), reply.size(), reading);
    std::string one = name(result);
    if (result == pz::Result::kOk) {
      one += " v=" + simio::fixed(reading.voltage_v, 2) + " i=" + simio::fixed(reading.current_a, 3) + " p=" + simio::fixed(reading.power_w, 1) + " e=" + simio::fixed(reading.energy_kwh, 3);
      if (model == pz::Model::kAc) one += " hz=" + simio::fixed(reading.frequency_hz, 1) + " pf=" + simio::fixed(reading.power_factor, 2);
      one += std::string(" alarm=") + (reading.alarm ? "1" : "0");
    }
    text += (first ? "" : "|") + one;
    first = false;
  }
  return text;
}

int main() {
  simio::Case c;
  while (simio::next_case(c)) std::cout << c.id << '\t' << (c.bus == BUS ? answer(c) : std::string("unsupported")) << '\n';
  return 0;
}
