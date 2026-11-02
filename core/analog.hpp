// ARMOR-NODE-ELECTRICAL - the analog inputs of the base board: what the ADC saw on a pin, made into a channel of the node's message.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// The firmware reads a pin many times (main/analog_inputs.cpp) and gives the samples, in millivolts, to this file. Nothing here touches hardware, so all of it is tested on a computer.
//   "dc": the average of the samples, minus the volts the sensor shows at zero, times the scale (a divider, or the amps a sensor gives per volt).
//   "ac": the effective (RMS) value of the samples around their own average, times the scale. Meant for a transformer or a clamp: a measure of the size of an alternating signal.
// A voltage is published as `voltage_v`; a current as `current_a`, and with a nominal voltage also as a power and an energy (the current times the voltage the circuit is assumed to have).
// The message's limits (a voltage is never negative, nothing leaves the contract's range) are kept here, so a wrong scale can never produce a message the server refuses.
#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "adc_stream.hpp"
#include "electrical_config.hpp"
#include "electrical_json.hpp"

namespace armor::analog {

inline double mean(const std::vector<double>& samples) {
  if (samples.empty()) return 0.0;
  double sum = 0.0;
  for (const double v : samples) sum += v;
  return sum / static_cast<double>(samples.size());
}

// The effective value of the signal around its own average.
inline double rms_about_mean(const std::vector<double>& samples) {
  if (samples.empty()) return 0.0;
  const double centre = mean(samples);
  double sum = 0.0;
  for (const double v : samples) sum += (v - centre) * (v - centre);
  return std::sqrt(sum / static_cast<double>(samples.size()));
}

// The calibration measured for this input (core/adc_stream.hpp); the identity until one is set.
inline adcstream::Calibration calibration_of(const config::AnalogSetting& input) { return adcstream::Calibration{input.cal_gain, input.cal_offset_mv}; }

// The quantity an input measures, in volts or amps, from the average and the effective value (around the average) of what the converter saw, in millivolts, as the chip's own calibration gave it.
// The input's calibration is applied here: an effective value only has the gain, the offset is the same on every sample and drops out of a value taken around the average.
inline double quantity_from(const config::AnalogSetting& input, double mean_mv, double rms_mv) {
  const adcstream::Calibration calibration = calibration_of(input);
  double value = 0.0;
  if (input.mode == "ac") value = rms_mv * calibration.gain / 1000.0 * std::fabs(input.scale);
  else value = (adcstream::apply(calibration, mean_mv) / 1000.0 - input.offset_v) * input.scale;
  if (std::fabs(value) < input.zero_below) value = 0.0;
  return value;
}

// The quantity an input measures, in volts or amps, from samples in millivolts.
inline double quantity_of(const config::AnalogSetting& input, const std::vector<double>& millivolts) {
  return quantity_from(input, mean(millivolts), rms_about_mean(millivolts));
}

// The energy a current input has made at its nominal voltage since the node started: it only grows.
class Energy {
 public:
  void add(double power_w, std::uint64_t milliseconds) {
    if (power_w > 0.0) kwh_ += power_w * static_cast<double>(milliseconds) / 3.6e9;
  }
  double kwh() const { return kwh_; }
 private:
  double kwh_ = 0.0;
};

inline double clamp(double value, double lowest, double highest) { return std::min(highest, std::max(lowest, value)); }

// One input's channel from the value it measured. `elapsed_ms` is the time since the last reading (for the energy).
inline void fill_channel(const config::AnalogSetting& input, double value, std::uint64_t elapsed_ms, Energy& energy, electrical::AnalogChannel& out) {
  out = electrical::AnalogChannel{};
  out.id = input.channel;
  out.label = input.label;
  out.ac = input.mode == "ac";
  if (input.quantity == "voltage") {
    out.has_voltage = true;
    out.voltage_v = clamp(value, 0.0, 1000.0);
  } else {
    out.has_current = true;
    out.current_a = clamp(value, -1000.0, 1000.0);
    if (input.nominal_voltage_v > 0.0) {
      const double power = out.current_a * input.nominal_voltage_v;
      energy.add(power, elapsed_ms);
      out.has_power = true;
      out.power_estimated = true;      // current times a nominal voltage: apparent power taken as watts
      out.power_w = clamp(power, -1000000.0, 1000000.0);
      out.has_energy = true;
      out.energy_kwh = clamp(energy.kwh(), 0.0, 10000000.0);
    }
  }
}

// One input's channel. A set of samples that is empty is no reading: the channel is not produced.
inline bool make_channel(const config::AnalogSetting& input, const std::vector<double>& millivolts, std::uint64_t elapsed_ms, Energy& energy, electrical::AnalogChannel& out) {
  if (millivolts.empty()) return false;
  fill_channel(input, quantity_of(input, millivolts), elapsed_ms, energy, out);
  return true;
}

// The same from a window read as a stream (core/adc_stream.hpp): the average and the effective value come from the Measurement, over whole cycles when it found them. The samples are the
// chip's, not yet calibrated: the input's calibration is applied in quantity_from.
inline bool make_channel_measured(const config::AnalogSetting& input, const adcstream::Measurement& measured, std::uint64_t elapsed_ms, Energy& energy, electrical::AnalogChannel& out) {
  if (!measured.valid) return false;
  fill_channel(input, quantity_from(input, measured.mean_mv, measured.rms_mv), elapsed_ms, energy, out);
  return true;
}

}  // namespace armor::analog
