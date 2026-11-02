// ARMOR-NODE-ELECTRICAL - the analog inputs read as a stream: the ADC fills a buffer by itself (DMA) at a fixed rate, the effective value is taken over WHOLE cycles of the mains found by their zero
// crossings, and each input has its own calibration (a gain and an offset measured against a reference).
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// The one-shot read (main/analog_inputs.cpp) takes samples one by one for a fixed window of 100 or 200 ms. That is a whole number of cycles of both 50 and 60 Hz only when the clock is exact and
// the mains is not drifting, and each read costs the CPU its time. Reading as a stream gives samples at an exact rate; this file finds where the cycles start and end in them and measures over
// whole cycles, so the effective value does not depend on where the window happened to begin. Nothing here touches hardware: the firmware gives it millivolts (main/analog_stream.cpp) and the tests
// give it an ADC that is simulated (tests/test_adc_stream.cpp), so the whole of it is tested on a computer.
//
// Everything that can be tuned is a setting (analog_stream in the node's settings, the calibration in each input), not a constant here.
#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace armor::adcstream {

// ---- settings ------------------------------------------------------------------------------------------------------------------

struct StreamSettings {
  bool continuous = false;     // off: the one-shot read, as the node has always done. On: the ADC runs by itself (DMA) while an input is read
  int sample_rate_hz = 20000;  // 2000..80000; the converter of the chip runs from 611 Hz to 83333 Hz in this mode
  int window_ms = 400;         // the longest an input is read for in one go (50..1000)
  int min_cycles = 5;          // whole cycles of the mains needed to measure over cycles (2..50); fewer and the plain window is used
  int hysteresis_mv = 30;      // the signal must go this far below its average before a rise through the average counts as a cycle (1..500)
  int min_hz = 40;             // a cycle shorter or longer than these is not the mains (20..100)
  int max_hz = 70;
};

constexpr int kMinSampleRate = 2000, kMaxSampleRate = 80000;
constexpr int kMinWindowMs = 50, kMaxWindowMs = 1000;
constexpr std::size_t kMaxWindowSamples = 16000;   // what one window may hold: the buffer of the firmware is this big, whatever the settings say

// The number of samples a window holds at these settings, never more than the buffer.
inline std::size_t window_samples(const StreamSettings& s) {
  const std::size_t wanted = static_cast<std::size_t>(s.sample_rate_hz) * static_cast<std::size_t>(s.window_ms) / 1000u;
  return std::min(wanted, kMaxWindowSamples);
}

// "" when the settings are fine, else the name of the member that is not.
inline const char* check(const StreamSettings& s) {
  if (s.sample_rate_hz < kMinSampleRate || s.sample_rate_hz > kMaxSampleRate) return "sample_rate_hz";
  if (s.window_ms < kMinWindowMs || s.window_ms > kMaxWindowMs) return "window_ms";
  if (s.min_cycles < 2 || s.min_cycles > 50) return "min_cycles";
  if (s.hysteresis_mv < 1 || s.hysteresis_mv > 500) return "hysteresis_mv";
  if (s.min_hz < 20 || s.min_hz > 100) return "min_hz";
  if (s.max_hz < 20 || s.max_hz > 100 || s.max_hz <= s.min_hz) return "max_hz";
  // the window has to hold at least the cycles that are asked for at the lowest frequency that counts as the mains
  const double window_s = static_cast<double>(window_samples(s)) / static_cast<double>(s.sample_rate_hz);
  if (window_s * s.min_hz < s.min_cycles) return "window_ms";
  return "";
}

// ---- calibration of one input --------------------------------------------------------------------------------------------------

// What the chip's own calibration gives (millivolts) is corrected with a gain and an offset measured against a reference: corrected = (reading - offset_mv) * gain.
struct Calibration {
  double gain = 1.0;
  double offset_mv = 0.0;
};
constexpr double kMinGain = 0.5, kMaxGain = 2.0, kMaxOffsetMv = 500.0;

inline bool valid(const Calibration& c) {
  return std::isfinite(c.gain) && std::isfinite(c.offset_mv) && c.gain >= kMinGain && c.gain <= kMaxGain && std::fabs(c.offset_mv) <= kMaxOffsetMv;
}
inline bool is_identity(const Calibration& c) { return c.gain == 1.0 && c.offset_mv == 0.0; }
inline double apply(const Calibration& c, double millivolts) { return (millivolts - c.offset_mv) * c.gain; }

// The calibration from two references: what the node read and what a meter said at a low level and at a high one. They have to be at least 100 mV apart in what the node read.
// False (and `out` unchanged) when they are not, or when the result is outside what a calibration may be (a gain of 0.5 to 2, an offset of at most 500 mV): such a number is a wrong
// reference or a broken input, not a calibration.
inline bool from_references(double read_low_mv, double true_low_mv, double read_high_mv, double true_high_mv, Calibration& out) {
  if (!std::isfinite(read_low_mv) || !std::isfinite(true_low_mv) || !std::isfinite(read_high_mv) || !std::isfinite(true_high_mv)) return false;
  if (read_high_mv - read_low_mv < 100.0 || true_high_mv <= true_low_mv) return false;
  Calibration made;
  made.gain = (true_high_mv - true_low_mv) / (read_high_mv - read_low_mv);
  made.offset_mv = read_low_mv - true_low_mv / made.gain;
  if (!valid(made)) return false;
  out = made;
  return true;
}
// The calibration of an alternating input, which has no offset (it is centred by itself): one reference level, the gain only.
inline bool gain_from_reference(double read_mv, double true_mv, Calibration& out) {
  if (!std::isfinite(read_mv) || !std::isfinite(true_mv) || read_mv < 10.0 || true_mv <= 0.0) return false;
  Calibration made;
  made.gain = true_mv / read_mv;
  if (!valid(made)) return false;
  out = made;
  return true;
}

// ---- measuring over whole cycles -----------------------------------------------------------------------------------------------

struct Measurement {
  bool valid = false;          // there was at least one sample
  bool whole_cycles = false;   // the effective value was taken over whole cycles of the mains (else over the whole window)
  int cycles = 0;
  double frequency_hz = 0.0;   // only with whole cycles
  double mean_mv = 0.0;
  double rms_mv = 0.0;         // around its own average, the way core/analog.hpp reads an "ac" input
  std::size_t samples_used = 0;
};

// The mean and the effective value (around the mean) of samples[first, last).
inline void mean_and_rms(const std::vector<double>& samples, std::size_t first, std::size_t last, double& mean, double& rms) {
  mean = rms = 0.0;
  if (last <= first) return;
  double sum = 0.0;
  for (std::size_t i = first; i < last; ++i) sum += samples[i];
  mean = sum / static_cast<double>(last - first);
  double squares = 0.0;
  for (std::size_t i = first; i < last; ++i) squares += (samples[i] - mean) * (samples[i] - mean);
  rms = std::sqrt(squares / static_cast<double>(last - first));
}

// The positions (in samples, with a fraction) where the signal rises through `centre` after having been `hysteresis` below it.
inline std::vector<double> rising_crossings(const std::vector<double>& samples, double centre, double hysteresis) {
  std::vector<double> found;
  bool armed = false;
  for (std::size_t i = 0; i < samples.size(); ++i) {
    const double v = samples[i];
    if (v < centre - hysteresis) armed = true;
    else if (armed && v >= centre && i > 0) {
      const double before = samples[i - 1];
      const double span = v - before;
      found.push_back(static_cast<double>(i - 1) + (span > 0.0 ? (centre - before) / span : 1.0));
      armed = false;
    }
  }
  return found;
}

// Measures a window of samples (millivolts, already calibrated). With enough whole cycles of a frequency that can be the mains, the mean and the effective value are those of
// the whole cycles; else (a direct signal, or a signal too small or too noisy to show its cycles) they are those of the whole window and `whole_cycles` says so.
inline Measurement measure(const std::vector<double>& samples, const StreamSettings& settings) {
  Measurement m;
  if (samples.empty()) return m;
  m.valid = true;
  double window_mean = 0.0, window_rms = 0.0;
  mean_and_rms(samples, 0, samples.size(), window_mean, window_rms);
  m.mean_mv = window_mean;
  m.rms_mv = window_rms;
  m.samples_used = samples.size();
  const std::vector<double> crossings = rising_crossings(samples, window_mean, static_cast<double>(settings.hysteresis_mv));
  if (crossings.size() < 2) return m;
  // the longest run of consecutive periods that are the period of a frequency that can be the mains
  const double shortest = static_cast<double>(settings.sample_rate_hz) / settings.max_hz;
  const double longest = static_cast<double>(settings.sample_rate_hz) / settings.min_hz;
  std::size_t best_start = 0, best_cycles = 0, run_start = 0;
  for (std::size_t k = 0; k + 1 < crossings.size(); ++k) {
    const double period = crossings[k + 1] - crossings[k];
    if (period < shortest || period > longest) { run_start = k + 1; continue; }
    const std::size_t cycles = k - run_start + 1;
    if (cycles > best_cycles) { best_cycles = cycles; best_start = run_start; }
  }
  if (best_cycles < static_cast<std::size_t>(settings.min_cycles)) return m;
  const double begin = crossings[best_start], end = crossings[best_start + best_cycles];
  const std::size_t first = static_cast<std::size_t>(std::ceil(begin));
  const std::size_t last = std::min(static_cast<std::size_t>(std::ceil(end)), samples.size());
  if (last <= first) return m;
  mean_and_rms(samples, first, last, m.mean_mv, m.rms_mv);
  m.whole_cycles = true;
  m.cycles = static_cast<int>(best_cycles);
  m.frequency_hz = static_cast<double>(settings.sample_rate_hz) * static_cast<double>(best_cycles) / (end - begin);
  m.samples_used = last - first;
  return m;
}

// ---- collecting the frames of the DMA ------------------------------------------------------------------------------------------

// The converter hands over frames of samples; a window is as many of them as the settings ask for. A frame that did not arrive (the driver reports an overrun: the reader was too slow and the
// buffer was overwritten) leaves a gap, which would put two parts of the signal side by side: the window is then marked and thrown away, never measured.
class WindowCollector {
 public:
  explicit WindowCollector(const StreamSettings& settings) : wanted_(window_samples(settings)) { samples_.reserve(wanted_); }

  // Adds samples (millivolts, calibrated). Returns true when the window is full. Samples beyond the window are dropped.
  bool add(const double* millivolts, std::size_t count) {
    for (std::size_t i = 0; i < count && samples_.size() < wanted_; ++i) samples_.push_back(millivolts[i]);
    return full();
  }
  // A frame was lost: the window can no longer be trusted.
  void note_overrun() { ++overruns_; broken_ = true; }
  bool full() const { return samples_.size() >= wanted_; }
  bool broken() const { return broken_; }
  unsigned overruns() const { return overruns_; }
  const std::vector<double>& samples() const { return samples_; }
  void restart() { samples_.clear(); broken_ = false; }   // the count of overruns is kept: it is a statistic of the stream

 private:
  std::size_t wanted_;
  std::vector<double> samples_;
  bool broken_ = false;
  unsigned overruns_ = 0;
};

}  // namespace armor::adcstream
