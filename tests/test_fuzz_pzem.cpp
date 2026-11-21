// ARMOR-NODE-ELECTRICAL - host tests with a fixed seed: the reader of the PZEM energy meters' replies (Modbus RTU, measurements and settings) must never crash, never accept a damaged
// reply and never turn a frame no real meter could send into a value, whatever arrives on the serial line.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <array>
#include <cstdint>
#include <cstdio>
#include <vector>

#include "../core/pzem.hpp"

static int failures = 0;
static int checks = 0;
#define CHECK(condition)                                                              \
  do {                                                                                \
    ++checks;                                                                         \
    if (!(condition)) {                                                               \
      ++failures;                                                                     \
      std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #condition);                \
    }                                                                                 \
  } while (0)

// the same numbers on every run, so a failure can be repeated
static std::uint32_t rng_state = 0x6C078965u;
static std::uint32_t next_random() {
  rng_state ^= rng_state << 13;
  rng_state ^= rng_state >> 17;
  rng_state ^= rng_state << 5;
  return rng_state;
}
static std::uint32_t below(std::uint32_t limit) { return limit == 0 ? 0 : next_random() % limit; }

using namespace armor::electrical::pzem;

static const Model kModels[2] = {Model::kAc, Model::kDc};

static void add_crc(std::vector<std::uint8_t>& frame) {
  const std::uint16_t crc = crc16(frame.data(), frame.size());
  frame.push_back(static_cast<std::uint8_t>(crc & 0xFF));
  frame.push_back(static_cast<std::uint8_t>(crc >> 8));
}

// a reply as a meter sends it: address, function, byte count, registers, CRC
static std::vector<std::uint8_t> reply(std::uint8_t address, std::uint8_t function, const std::vector<std::uint16_t>& registers) {
  std::vector<std::uint8_t> frame = {address, function, static_cast<std::uint8_t>(registers.size() * 2)};
  for (std::uint16_t value : registers) {
    frame.push_back(static_cast<std::uint8_t>(value >> 8));
    frame.push_back(static_cast<std::uint8_t>(value & 0xFF));
  }
  add_crc(frame);
  return frame;
}

static std::vector<std::uint16_t> plausible_registers(Model model) {
  std::vector<std::uint16_t> r;
  if (model == Model::kAc) {   // voltage, current (2), power (2), energy (2), frequency, power factor, alarm
    r = {static_cast<std::uint16_t>(2000 + below(500)), static_cast<std::uint16_t>(below(40000)), 0, static_cast<std::uint16_t>(below(40000)), 0,
         static_cast<std::uint16_t>(below(60000)), 0, static_cast<std::uint16_t>(480 + below(40)), static_cast<std::uint16_t>(below(101)), static_cast<std::uint16_t>(below(2) ? 0xFFFF : 0)};
  } else {   // voltage, current, power (2), energy (2), the two alarms
    r = {static_cast<std::uint16_t>(below(30000)), static_cast<std::uint16_t>(below(10000)), static_cast<std::uint16_t>(below(40000)), 0,
         static_cast<std::uint16_t>(below(60000)), 0, static_cast<std::uint16_t>(below(2) ? 0xFFFF : 0), 0};
  }
  return r;
}

static void requests() {
  // only the addresses a meter can have are asked, for both requests of both meters
  for (Model model : kModels) {
    for (unsigned address = 0; address < 256; ++address) {
      std::array<std::uint8_t, kRequestLength> out{};
      const bool allowed = (address >= 1 && address <= 247) || address == 0xF8;
      CHECK(build_read_request(model, static_cast<std::uint8_t>(address), out) == allowed);
      if (allowed) CHECK(out[0] == static_cast<std::uint8_t>(address) && out[1] == 0x04 && crc16(out.data(), 6) == (out[6] | (out[7] << 8)));
      CHECK(build_holding_request(model, static_cast<std::uint8_t>(address), out) == allowed);
      if (allowed) CHECK(out[0] == static_cast<std::uint8_t>(address) && out[1] == 0x03 && crc16(out.data(), 6) == (out[6] | (out[7] << 8)));
    }
  }
}

static void whole_replies() {
  for (int round = 0; round < 1000; ++round) {
    for (Model model : kModels) {
      const std::uint8_t address = static_cast<std::uint8_t>(1 + below(247));
      const std::vector<std::uint8_t> frame = reply(address, 0x04, plausible_registers(model));
      CHECK(frame.size() == reply_length(model));
      Reading reading;
      CHECK(decode(model, address, frame.data(), frame.size(), reading) == Result::kOk);
      CHECK(reading.voltage_v >= 0 && reading.voltage_v <= 1000);
      // the meter on another address is not this one, unless the line is asked as a broadcast
      Reading other;
      CHECK(decode(model, static_cast<std::uint8_t>(address == 1 ? 2 : address - 1), frame.data(), frame.size(), other) == Result::kAddress);
      CHECK(decode(model, 0xF8, frame.data(), frame.size(), other) == Result::kOk);
    }
  }
}

static void damaged_replies() {
  for (int round = 0; round < 600; ++round) {
    for (Model model : kModels) {
      const std::uint8_t address = static_cast<std::uint8_t>(1 + below(247));
      const std::vector<std::uint8_t> frame = reply(address, 0x04, plausible_registers(model));
      // one changed byte is always seen (the CRC), wherever it is
      std::vector<std::uint8_t> bad = frame;
      const std::size_t at = below(static_cast<std::uint32_t>(bad.size()));
      bad[at] = static_cast<std::uint8_t>(bad[at] + 1 + below(255));
      Reading reading;
      CHECK(decode(model, address, bad.data(), bad.size(), reading) != Result::kOk);
      // a frame cut short, or with bytes added, is not a reply
      const std::size_t cut = below(static_cast<std::uint32_t>(frame.size()));
      CHECK(decode(model, address, frame.data(), cut, reading) != Result::kOk);
      std::vector<std::uint8_t> longer = frame;
      longer.push_back(static_cast<std::uint8_t>(below(256)));
      CHECK(decode(model, address, longer.data(), longer.size(), reading) != Result::kOk);
    }
  }
}

static void exceptions() {
  for (int round = 0; round < 300; ++round) {
    const std::uint8_t address = static_cast<std::uint8_t>(1 + below(247));
    std::vector<std::uint8_t> failure = {address, static_cast<std::uint8_t>(0x04 | 0x80), static_cast<std::uint8_t>(1 + below(4))};
    add_crc(failure);
    Reading reading;
    CHECK(decode(Model::kAc, address, failure.data(), failure.size(), reading) == Result::kException);
    CHECK(decode(Model::kDc, 0xF8, failure.data(), failure.size(), reading) == Result::kException);
    CHECK(decode(Model::kAc, static_cast<std::uint8_t>(address == 1 ? 2 : address - 1), failure.data(), failure.size(), reading) == Result::kAddress);
    std::vector<std::uint8_t> holding_failure = {address, static_cast<std::uint8_t>(0x03 | 0x80), 2};
    add_crc(holding_failure);
    Holding holding;
    CHECK(decode_holding(Model::kDc, address, holding_failure.data(), holding_failure.size(), holding) == Result::kException);
    CHECK(decode(Model::kAc, address, holding_failure.data(), holding_failure.size(), reading) != Result::kOk);
  }
}

static void noise() {
  for (int round = 0; round < 6000; ++round) {
    std::vector<std::uint8_t> bytes;
    for (std::uint32_t i = below(40); i > 0; --i) bytes.push_back(static_cast<std::uint8_t>(below(256)));
    const std::uint8_t address = static_cast<std::uint8_t>(below(256));
    for (Model model : kModels) {
      Reading reading;
      Holding holding;
      (void)decode(model, address, bytes.data(), bytes.size(), reading);
      (void)decode_holding(model, address, bytes.data(), bytes.size(), holding);
    }
  }
  // noise of the right length and a true CRC: only the content can refuse it, and what is accepted is a value a meter could give
  for (int round = 0; round < 4000; ++round) {
    for (Model model : kModels) {
      std::vector<std::uint8_t> body = {static_cast<std::uint8_t>(1 + below(247)), static_cast<std::uint8_t>(below(4) == 0 ? below(256) : 0x04), static_cast<std::uint8_t>(2 * register_count(model))};
      for (std::size_t i = 0; i < 2 * register_count(model); ++i) body.push_back(static_cast<std::uint8_t>(below(256)));
      add_crc(body);
      Reading reading;
      const Result result = decode(model, body[0], body.data(), body.size(), reading);
      if (result == Result::kOk) {
        CHECK(body[1] == 0x04 && reading.voltage_v <= 1000 && reading.current_a <= 1000 && reading.power_w <= 1'000'000 && reading.energy_kwh <= 10'000'000);
        CHECK(reading.frequency_hz <= 100 && reading.power_factor <= 1.0);
      } else {
        CHECK(result == Result::kFunction || result == Result::kRange || result == Result::kException || result == Result::kLength);
      }

      std::vector<std::uint8_t> settings = {static_cast<std::uint8_t>(1 + below(247)), 0x03, static_cast<std::uint8_t>(2 * holding_count(model))};
      for (std::size_t i = 0; i < 2 * holding_count(model); ++i) settings.push_back(static_cast<std::uint8_t>(below(4) == 0 ? below(256) : 0));
      add_crc(settings);
      Holding holding;
      if (decode_holding(model, settings[0], settings.data(), settings.size(), holding) == Result::kOk) {
        CHECK(holding.address >= 1 && holding.address <= 247);
        CHECK(model == Model::kAc ? holding.has_power_alarm : (holding.has_shunt && holding.shunt_a >= 50 && holding.shunt_a <= 300));
      }
    }
  }
}

int main() {
  requests();
  whole_replies();
  damaged_replies();
  exceptions();
  noise();
  std::printf("fuzz_pzem: %d checks, %d failed\n", checks, failures);
  return failures == 0 ? 0 : 1;
}
