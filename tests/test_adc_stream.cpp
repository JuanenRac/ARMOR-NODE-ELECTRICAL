// ARMOR-NODE-ELECTRICAL - host tests of the analog inputs read as a stream: the effective value over whole cycles of the mains, the calibration of each input and the frames of the DMA, on an
// ADC that is simulated (its rate, its resolution, the error of its front end, noise, and the frames it hands over).
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

#include "../core/adc_stream.hpp"
#include "../core/analog.hpp"
#include "../core/electrical_config.hpp"

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

using namespace armor;

static bool near(double a, double b, double tolerance) { return std::fabs(a - b) <= tolerance; }
static bool within_percent(double value, double expected, double percent) { return std::fabs(value - expected) <= std::fabs(expected) * percent / 100.0; }

// ---- the simulated converter ---------------------------------------------------------------------------------------------------------

// A 12-bit converter that reads 0 to 3100 mV, fed by a signal on a pin (millivolts as a function of time). `gain_error` and `offset_error_mv` are the faults of the front end that a
// calibration has to take out. Samples are taken at a fixed rate, one after another, so the converter can be asked for frames the way the DMA hands them over.
class SimAdc {
 public:
  SimAdc(double rate_hz, std::function<double(double)> signal) : rate_(rate_hz), signal_(std::move(signal)) {}
  double gain_error = 1.0, offset_error_mv = 0.0, noise_mv = 0.0;

  std::vector<double> frame(std::size_t samples) {
    std::vector<double> out;
    out.reserve(samples);
    for (std::size_t i = 0; i < samples; ++i) {
      const double t = static_cast<double>(taken_++) / rate_;
      double v = signal_(t) * gain_error + offset_error_mv + noise();
      const double raw = std::min(4095.0, std::max(0.0, std::round(v / 3100.0 * 4095.0)));
      out.push_back(raw * 3100.0 / 4095.0);
    }
    return out;
  }
  void skip(std::size_t samples) { taken_ += samples; }   // a frame the reader never saw

 private:
  double noise() {
    if (noise_mv == 0.0) return 0.0;
    state_ = state_ * 1664525u + 1013904223u;
    return (static_cast<double>(state_ >> 8) / 16777216.0 - 0.5) * 2.0 * noise_mv;
  }
  double rate_;
  std::function<double(double)> signal_;
  std::uint64_t taken_ = 0;
  std::uint32_t state_ = 12345;
};

static std::function<double(double)> mains(double hz, double amplitude_mv, double centre_mv = 1550.0, double third = 0.0, double phase = 0.7) {
  return [=](double t) { return centre_mv + amplitude_mv * (std::sin(2.0 * M_PI * hz * t + phase) + third * std::sin(2.0 * M_PI * 3.0 * hz * t)); };
}

// Takes a window the way the firmware does: frames from the converter until the collector is full.
static std::vector<double> read_window(SimAdc& adc, const adcstream::StreamSettings& s, std::size_t frame = 256) {
  adcstream::WindowCollector collector(s);
  while (!collector.full()) {
    const std::vector<double> got = adc.frame(frame);
    collector.add(got.data(), got.size());
  }
  return collector.samples();
}

// ---- tests ---------------------------------------------------------------------------------------------------------------------------

static void test_settings() {
  adcstream::StreamSettings s;
  CHECK(!s.continuous && std::string(adcstream::check(s)).empty());   // off until it is asked for
  CHECK(adcstream::window_samples(s) == 8000);
  const auto bad = [](adcstream::StreamSettings x, const char* member) { return std::string(adcstream::check(x)) == member; };
  { auto x = s; x.sample_rate_hz = 1999; CHECK(bad(x, "sample_rate_hz")); }
  { auto x = s; x.sample_rate_hz = 80001; CHECK(bad(x, "sample_rate_hz")); }
  { auto x = s; x.window_ms = 49; CHECK(bad(x, "window_ms")); }
  { auto x = s; x.window_ms = 1001; CHECK(bad(x, "window_ms")); }
  { auto x = s; x.min_cycles = 1; CHECK(bad(x, "min_cycles")); }
  { auto x = s; x.hysteresis_mv = 0; CHECK(bad(x, "hysteresis_mv")); }
  { auto x = s; x.min_hz = 10; CHECK(bad(x, "min_hz")); }
  { auto x = s; x.max_hz = 40; CHECK(bad(x, "max_hz")); }   // not above the lowest
  { auto x = s; x.window_ms = 50; CHECK(bad(x, "window_ms")); }   // 50 ms hold two cycles at 40 Hz, five are asked for
  { auto x = s; x.sample_rate_hz = 80000; x.window_ms = 1000; CHECK(adcstream::window_samples(x) == adcstream::kMaxWindowSamples); }   // the buffer of the firmware is the limit
}

static void test_calibration() {
  using adcstream::Calibration;
  Calibration c;
  CHECK(adcstream::is_identity(c) && adcstream::valid(c) && near(adcstream::apply(c, 1234.5), 1234.5, 1e-12));
  // a front end that reads 3 % high and 12 mV over: two references taken with a meter take it out
  SimAdc adc(20000.0, [](double) { return 500.0; });
  adc.gain_error = 1.03; adc.offset_error_mv = 12.0;
  const double read_low = adc.frame(64).back();
  SimAdc adc_high(20000.0, [](double) { return 2500.0; });
  adc_high.gain_error = 1.03; adc_high.offset_error_mv = 12.0;
  const double read_high = adc_high.frame(64).back();
  CHECK(adcstream::from_references(read_low, 500.0, read_high, 2500.0, c));
  CHECK(near(c.gain, 1.0 / 1.03, 0.002) && adcstream::valid(c) && !adcstream::is_identity(c));
  CHECK(near(adcstream::apply(c, read_low), 500.0, 1.0) && near(adcstream::apply(c, read_high), 2500.0, 1.0));
  // the calibrated reading is right in between, too
  SimAdc mid(20000.0, [](double) { return 1500.0; });
  mid.gain_error = 1.03; mid.offset_error_mv = 12.0;
  CHECK(near(adcstream::apply(c, mid.frame(8).back()), 1500.0, 1.5));
  // references that cannot be a calibration leave the old one alone
  const Calibration kept = c;
  CHECK(!adcstream::from_references(500.0, 500.0, 550.0, 2500.0, c) && c.gain == kept.gain);        // less than 100 mV apart
  CHECK(!adcstream::from_references(500.0, 500.0, 2500.0, 400.0, c) && c.gain == kept.gain);        // the true values go the other way
  CHECK(!adcstream::from_references(500.0, 500.0, 2500.0, 9000.0, c) && c.gain == kept.gain);       // a gain of 4.4: a wrong reference
  CHECK(!adcstream::from_references(500.0, 500.0, 2500.0, 600.0, c) && c.gain == kept.gain);        // a gain of 0.05
  CHECK(!adcstream::from_references(NAN, 500.0, 2500.0, 2500.0, c) && c.gain == kept.gain);
  CHECK(!adcstream::from_references(0.0, 900.0, 2500.0, 2500.0, c) && c.gain == kept.gain);        // an offset beyond 500 mV
  Calibration a;
  CHECK(adcstream::gain_from_reference(700.0, 707.0, a) && near(a.gain, 1.01, 1e-9) && a.offset_mv == 0.0);
  CHECK(!adcstream::gain_from_reference(5.0, 707.0, a) && !adcstream::gain_from_reference(700.0, 0.0, a) && !adcstream::gain_from_reference(700.0, 3000.0, a));
  CHECK(!adcstream::valid(Calibration{0.4, 0.0}) && !adcstream::valid(Calibration{2.1, 0.0}) && !adcstream::valid(Calibration{1.0, 501.0}) && !adcstream::valid(Calibration{NAN, 0.0}) && adcstream::valid(Calibration{0.5, -500.0}));
}

static void test_whole_cycles_beat_a_plain_window() {
  adcstream::StreamSettings s;
  s.window_ms = 130;   // six and a half cycles of 50 Hz: a plain window ends in the middle of a cycle
  s.min_cycles = 5;
  const double expected_rms = 1000.0 / std::sqrt(2.0);
  double worst_plain = 0.0, worst_whole = 0.0;
  int cases = 0;
  for (const double hz : {50.0, 60.0, 49.2, 51.0}) {
    for (const std::size_t skip : {0u, 37u, 137u, 251u}) {   // the window does not begin where a cycle does
      SimAdc adc(s.sample_rate_hz, mains(hz, 1000.0));
      adc.noise_mv = 3.0;
      adc.skip(skip);
      const std::vector<double> window = read_window(adc, s);
      const adcstream::Measurement m = adcstream::measure(window, s);
      CHECK(m.valid && m.whole_cycles && m.cycles >= 5);
      CHECK(within_percent(m.rms_mv, expected_rms, 0.3));
      CHECK(near(m.frequency_hz, hz, 0.15));
      CHECK(near(m.mean_mv, 1550.0, 6.0));
      // what the plain window gives for the same samples, which the stream replaces
      double plain_mean = 0.0, plain_rms = 0.0;
      adcstream::mean_and_rms(window, 0, window.size(), plain_mean, plain_rms);
      worst_plain = std::max(worst_plain, std::fabs(plain_rms - expected_rms) / expected_rms);
      worst_whole = std::max(worst_whole, std::fabs(m.rms_mv - expected_rms) / expected_rms);
      ++cases;
    }
  }
  CHECK(cases == 16);
  CHECK(worst_plain > 0.006);                 // the plain window is off by more than half a percent in some of these (about one percent at 51 Hz)
  CHECK(worst_whole < 0.003);                 // the whole cycles never by more than 0.3 %, noise and the 12 bits of the converter included
  CHECK(worst_whole < worst_plain / 3.0);
}

static void test_other_signals() {
  adcstream::StreamSettings s;
  // a third harmonic (a distorted current): the cycles are still the cycles
  SimAdc distorted(s.sample_rate_hz, mains(50.0, 800.0, 1550.0, 0.3));
  distorted.noise_mv = 2.0;
  adcstream::Measurement m = adcstream::measure(read_window(distorted, s), s);
  const double expected = 800.0 * std::sqrt((1.0 + 0.09) / 2.0);
  CHECK(m.whole_cycles && within_percent(m.rms_mv, expected, 0.5) && near(m.frequency_hz, 50.0, 0.15));
  // a direct signal has no cycles: the whole window, its mean, and no pretence of a frequency
  SimAdc direct(s.sample_rate_hz, [](double) { return 1234.0; });
  direct.noise_mv = 2.0;
  m = adcstream::measure(read_window(direct, s), s);
  CHECK(m.valid && !m.whole_cycles && m.cycles == 0 && m.frequency_hz == 0.0 && near(m.mean_mv, 1234.0, 2.0) && m.rms_mv < 5.0 && m.samples_used == adcstream::window_samples(s));
  // noise smaller than the hysteresis around a flat level is not a cycle
  SimAdc quiet(s.sample_rate_hz, [](double) { return 900.0; });
  quiet.noise_mv = 12.0;
  m = adcstream::measure(read_window(quiet, s), s);
  CHECK(!m.whole_cycles);
  // a slow signal (5 Hz) and a fast one (300 Hz) are not the mains: the plain window is used
  for (const double hz : {5.0, 300.0}) {
    SimAdc other(s.sample_rate_hz, mains(hz, 900.0));
    m = adcstream::measure(read_window(other, s), s);
    CHECK(m.valid && !m.whole_cycles);
  }
  // a small current clamp signal (20 mV of swing) under the hysteresis shows no cycles; with the hysteresis lowered by the setting it does
  SimAdc small(s.sample_rate_hz, mains(50.0, 20.0));
  small.noise_mv = 1.0;
  const std::vector<double> window = read_window(small, s);
  CHECK(!adcstream::measure(window, s).whole_cycles);
  adcstream::StreamSettings sensitive = s;
  sensitive.hysteresis_mv = 10;
  m = adcstream::measure(window, sensitive);
  CHECK(m.whole_cycles && within_percent(m.rms_mv, 20.0 / std::sqrt(2.0), 8.0));
  // the minimum number of cycles is a setting: the same window with 30 asked for falls back (only 20 fit)
  adcstream::StreamSettings strict = s;
  strict.min_cycles = 30;
  SimAdc twenty(s.sample_rate_hz, mains(50.0, 1000.0));
  CHECK(!adcstream::measure(read_window(twenty, s), strict).whole_cycles);
  // nothing in, nothing out
  CHECK(!adcstream::measure({}, s).valid);
  CHECK(adcstream::measure({42.0}, s).valid && !adcstream::measure({42.0}, s).whole_cycles && adcstream::measure({42.0}, s).mean_mv == 42.0);
}

static void test_calibrated_chain() {
  // an input whose front end reads 3 % high: with the calibration measured for it, an AC current of known size comes out right
  config::AnalogSetting input;
  input.enabled = true; input.quantity = "current"; input.mode = "ac"; input.scale = 30.0;   // 30 A per volt
  adcstream::StreamSettings s;
  SimAdc adc(s.sample_rate_hz, mains(50.0, 500.0));   // 354 mV effective: 10.6 A
  adc.gain_error = 1.03;
  const adcstream::Measurement m = adcstream::measure(read_window(adc, s), s);
  analog::Energy energy;
  electrical::AnalogChannel channel;
  CHECK(analog::make_channel_measured(input, m, 0, energy, channel) && channel.has_current);
  const double truth = 500.0 / std::sqrt(2.0) / 1000.0 * 30.0;
  CHECK(within_percent(channel.current_a, truth * 1.03, 0.6));   // uncalibrated: the 3 % shows
  input.cal_gain = 1.0 / 1.03;
  CHECK(analog::make_channel_measured(input, m, 0, energy, channel) && within_percent(channel.current_a, truth, 0.6));
  // a DC input: the offset of the front end is taken out too
  config::AnalogSetting dc;
  dc.enabled = true; dc.quantity = "voltage"; dc.mode = "dc"; dc.scale = 4.0;   // a 4:1 divider
  SimAdc flat(s.sample_rate_hz, [](double) { return 2000.0; });
  flat.gain_error = 1.02; flat.offset_error_mv = 15.0;
  const adcstream::Measurement dm = adcstream::measure(read_window(flat, s), s);
  CHECK(analog::make_channel_measured(dc, dm, 0, energy, channel) && channel.has_voltage && within_percent(channel.voltage_v, 8.0 * 1.02 + 0.06, 0.4));
  dc.cal_gain = 1.0 / 1.02; dc.cal_offset_mv = 15.0;
  CHECK(analog::make_channel_measured(dc, dm, 0, energy, channel) && within_percent(channel.voltage_v, 8.0, 0.4));
  // the same as the one-shot path gives on the same samples (the calibration is in both)
  const std::vector<double> samples = read_window(flat, s);
  electrical::AnalogChannel one_shot;
  CHECK(analog::make_channel(dc, samples, 0, energy, one_shot) && near(one_shot.voltage_v, channel.voltage_v, 0.01));
  CHECK(!analog::make_channel_measured(dc, adcstream::Measurement{}, 0, energy, channel));
  // without a calibration nothing changed from before it existed
  config::AnalogSetting plain;
  plain.enabled = true; plain.scale = 2.0;
  CHECK(near(analog::quantity_of(plain, {1000.0, 1000.0, 1000.0}), 2.0, 1e-12));
}

static void test_frames_and_overruns() {
  adcstream::StreamSettings s;
  adcstream::WindowCollector collector(s);
  SimAdc adc(s.sample_rate_hz, mains(50.0, 1000.0));
  std::size_t frames = 0;
  while (!collector.full()) { const std::vector<double> got = adc.frame(256); collector.add(got.data(), got.size()); ++frames; }
  CHECK(frames == 32 && collector.samples().size() == 8000 && !collector.broken());   // 31.25 frames of 256: the last is cut
  // a window is never longer than asked for
  const std::vector<double> more = adc.frame(256);
  collector.add(more.data(), more.size());
  CHECK(collector.samples().size() == 8000);
  // a lost frame breaks the window; the statistic of overruns survives the restart
  collector.restart();
  CHECK(collector.samples().empty() && !collector.broken());
  const std::vector<double> first = adc.frame(256);
  collector.add(first.data(), first.size());
  collector.note_overrun();
  CHECK(collector.broken() && collector.overruns() == 1);
  collector.restart();
  CHECK(!collector.broken() && collector.overruns() == 1);
  // frames of any size give the same window
  for (const std::size_t frame : {1u, 7u, 64u, 255u, 1000u, 8000u}) {
    SimAdc a(s.sample_rate_hz, mains(50.0, 1000.0));
    SimAdc b(s.sample_rate_hz, mains(50.0, 1000.0));
    CHECK(read_window(a, s, frame) == read_window(b, s, 256));
  }
  // the biggest window the firmware can hold, at the highest rate
  adcstream::StreamSettings big;
  big.sample_rate_hz = 80000; big.window_ms = 1000;
  SimAdc fast(big.sample_rate_hz, mains(50.0, 1000.0));
  const adcstream::Measurement m = adcstream::measure(read_window(fast, big), big);
  CHECK(m.samples_used <= adcstream::kMaxWindowSamples && m.whole_cycles && near(m.frequency_hz, 50.0, 0.5));
}

static void test_document() {
  config::Settings s = config::default_settings("a1b2c3");
  s.sta.enabled = true; s.sta.ssid = "casa"; s.sta.password = "una-clave-larga";
  CHECK(!s.analog_stream.continuous && s.analogs[0].cal_gain == 1.0 && s.analogs[0].cal_offset_mv == 0.0 && config::validate(s).empty());
  s.analogs[0].enabled = true; s.analogs[0].channel = "mains"; s.analogs[0].pin = 1;
  s.analogs[0].cal_gain = 0.972; s.analogs[0].cal_offset_mv = 11.5;
  s.analog_stream.continuous = true; s.analog_stream.sample_rate_hz = 40000; s.analog_stream.window_ms = 300; s.analog_stream.min_cycles = 8; s.analog_stream.hysteresis_mv = 25; s.analog_stream.min_hz = 45; s.analog_stream.max_hz = 65;
  CHECK(config::validate(s).empty());
  const std::string text = config::to_json(s, true);
  config::Settings back;
  config::Problems problems;
  CHECK(config::load(text, config::default_settings("a1b2c3"), back, problems) && problems.empty());
  CHECK(back.analog_stream.continuous && back.analog_stream.sample_rate_hz == 40000 && back.analog_stream.window_ms == 300 && back.analog_stream.min_cycles == 8 && back.analog_stream.hysteresis_mv == 25 && back.analog_stream.min_hz == 45 && back.analog_stream.max_hz == 65);
  CHECK(near(back.analogs[0].cal_gain, 0.972, 1e-9) && near(back.analogs[0].cal_offset_mv, 11.5, 1e-9) && config::to_json(back, true) == text);
  // a document that does not mention them leaves them as they were; one that does is checked by name
  config::Settings partial;
  problems.clear();
  CHECK(config::load("{\"analog_stream\":{\"window_ms\":500}}", s, partial, problems) && partial.analog_stream.window_ms == 500 && partial.analog_stream.continuous && near(partial.analogs[0].cal_gain, 0.972, 1e-9));
  const auto has = [](const config::Problems& list, const std::string& path, const std::string& code) { for (const config::Problem& p : list) if (p.path == path && p.code == code) return true; return false; };
  problems.clear();
  CHECK(!config::load("{\"analog_stream\":{\"sample_rate_hz\":100}}", s, partial, problems) && has(problems, "analog_stream.sample_rate_hz", "range"));
  problems.clear();
  CHECK(!config::load("{\"analog_stream\":{\"continuous\":\"yes\"}}", s, partial, problems) && has(problems, "analog_stream.continuous", "invalid"));
  problems.clear();
  CHECK(!config::load("{\"analog_stream\":{\"window_ms\":50,\"min_cycles\":10}}", s, partial, problems) && has(problems, "analog_stream.window_ms", "range"));   // the window cannot hold the cycles asked for
  problems.clear();
  CHECK(!config::load("{\"analogs\":[{\"cal_gain\":3.0}]}", s, partial, problems) && has(problems, "analogs.0.cal_gain", "range"));
  problems.clear();
  CHECK(!config::load("{\"analogs\":[{\"cal_offset_mv\":900}]}", s, partial, problems) && has(problems, "analogs.0.cal_offset_mv", "range"));
  problems.clear();
  CHECK(!config::load("{\"analogs\":[{\"mode\":\"ac\",\"cal_offset_mv\":5}]}", s, partial, problems) && has(problems, "analogs.0.cal_offset_mv", "invalid"));   // an alternating signal has no offset
  problems.clear();
  CHECK(!config::load("{\"analogs\":[{\"cal_gain\":\"big\"}]}", s, partial, problems) && has(problems, "analogs.0.cal_gain", "invalid"));
  // a document from before these existed loads with the defaults
  problems.clear();
  config::Settings old;
  CHECK(config::load("{\"v\":1,\"node\":{\"id\":\"electrical-a1b2c3\",\"name\":\"x\"},\"sta\":{\"enabled\":true,\"ssid\":\"casa\",\"password\":\"una-clave-larga\"}}", config::default_settings("a1b2c3"), old, problems) && !old.analog_stream.continuous && old.analogs[0].cal_gain == 1.0);
}

int main() {
  test_settings();
  test_calibration();
  test_whole_cycles_beat_a_plain_window();
  test_other_signals();
  test_calibrated_chain();
  test_frames_and_overruns();
  test_document();
  std::printf("%d checks, %d failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}
