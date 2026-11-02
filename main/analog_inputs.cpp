// ARMOR-NODE-ELECTRICAL - the analog inputs of the base board. Nothing here has run on a board.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// One task reads the enabled inputs in turn. A pin is sampled as fast as the converter allows for a fixed window (a whole number of mains periods for both 50 and 60 Hz: 200 ms for an
// effective value, 100 ms for an average), the samples are converted to millivolts with the chip's own calibration, and core/analog.hpp makes the quantity and the channel. Only ADC1 is
// used: the converter shared with the Wi-Fi radio is never touched. The input range is 0 to about 3.1 V (12 dB attenuation); what is wired to a pin must never go beyond 3.3 V.
#include "analog_inputs.hpp"

#include <mutex>
extern "C" {
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
}
#include "core/adc_stream.hpp"
#include "core/analog.hpp"
#include "entropy.hpp"

namespace armor::analoginputs {
namespace {
constexpr char kTag[] = "armor-analog";
constexpr std::uint64_t kFreshMs = 15000;   // a reading older than this is not in the message

std::mutex g_lock;
config::Settings g_settings;
std::array<InputStatus, board::kAnalogCount> g_status;
std::array<electrical::AnalogChannel, board::kAnalogCount> g_channels;
std::array<std::uint64_t, board::kAnalogCount> g_read_at_ms{};
std::array<bool, board::kAnalogCount> g_have{};

std::uint64_t now_ms() { return static_cast<std::uint64_t>(esp_timer_get_time()) / 1000ULL; }

void analog_task(void*) {
  adc_oneshot_unit_handle_t unit = nullptr;
  adc_oneshot_unit_init_cfg_t unit_config{};
  unit_config.unit_id = ADC_UNIT_1;
  if (adc_oneshot_new_unit(&unit_config, &unit) != ESP_OK) {
    ESP_LOGE(kTag, "the converter could not be started: the analog inputs are not read");
    std::lock_guard<std::mutex> guard(g_lock);
    for (std::size_t i = 0; i < board::kAnalogCount; ++i) if (g_settings.analogs[i].enabled) { g_status[i].state = "error"; g_status[i].error = "adc"; }
    vTaskDelete(nullptr);
    return;
  }
  std::array<adc_cali_handle_t, board::kAnalogCount> calibration{};
  std::array<bool, board::kAnalogCount> ready{};
  std::array<analog::Energy, board::kAnalogCount> energy;
  std::array<std::uint64_t, board::kAnalogCount> last_ms{};
  for (std::size_t i = 0; i < board::kAnalogCount; ++i) {
    const config::AnalogSetting& input = g_settings.analogs[i];
    if (!input.enabled) continue;
    const adc_channel_t channel = static_cast<adc_channel_t>(board::adc1_channel(input.pin));
    adc_oneshot_chan_cfg_t channel_config{};
    channel_config.atten = ADC_ATTEN_DB_12;
    channel_config.bitwidth = ADC_BITWIDTH_DEFAULT;
    adc_cali_curve_fitting_config_t cali_config{};
    cali_config.unit_id = ADC_UNIT_1;
    cali_config.chan = channel;
    cali_config.atten = ADC_ATTEN_DB_12;
    cali_config.bitwidth = ADC_BITWIDTH_DEFAULT;
    if (adc_oneshot_config_channel(unit, channel, &channel_config) == ESP_OK && adc_cali_create_scheme_curve_fitting(&cali_config, &calibration[i]) == ESP_OK) {
      ready[i] = true;
      std::lock_guard<std::mutex> guard(g_lock);
      g_status[i].state = "waiting";
    } else {
      ESP_LOGE(kTag, "GPIO %d could not be set up as an analog input", input.pin);
      std::lock_guard<std::mutex> guard(g_lock);
      g_status[i].state = "error";
      g_status[i].error = "adc";
    }
  }
  std::vector<double> millivolts;
  millivolts.reserve(8000);
  esp_task_wdt_add(nullptr);   // a task that hangs here would leave the readings to go stale unseen: the watchdog restarts the board
  for (;;) {
    esp_task_wdt_reset();
    bool any = false;
    for (std::size_t i = 0; i < board::kAnalogCount; ++i) {
      const config::AnalogSetting& input = g_settings.analogs[i];
      if (!input.enabled || !ready[i]) continue;
      any = true;
      const adc_channel_t channel = static_cast<adc_channel_t>(board::adc1_channel(input.pin));
      const std::uint64_t window_us = input.mode == "ac" ? 200000ULL : 100000ULL;
      millivolts.clear();
      {
        std::lock_guard<std::mutex> adc(armor::adc_lock());   // nobody else touches the converter during a window (see entropy.cpp): a login must not spoil it
        const std::uint64_t started = static_cast<std::uint64_t>(esp_timer_get_time());
        while (static_cast<std::uint64_t>(esp_timer_get_time()) - started < window_us && millivolts.size() < 8000) {
          int raw = 0, mv = 0;
          if (adc_oneshot_read(unit, channel, &raw) == ESP_OK && adc_cali_raw_to_voltage(calibration[i], raw, &mv) == ESP_OK) millivolts.push_back(static_cast<double>(mv));
        }
      }
      const std::uint64_t at = now_ms();
      const std::uint64_t elapsed = last_ms[i] == 0 ? 0 : at - last_ms[i];
      last_ms[i] = at;
      electrical::AnalogChannel made;
      if (analog::make_channel(input, millivolts, elapsed, energy[i], made)) {
        std::lock_guard<std::mutex> guard(g_lock);
        g_channels[i] = made;
        g_read_at_ms[i] = at;
        g_have[i] = true;
        g_status[i].state = "reading";
        g_status[i].error.clear();
        g_status[i].millivolts = analog::mean(millivolts);
        g_status[i].value = analog::quantity_of(input, millivolts);
        g_status[i].samples = static_cast<std::uint32_t>(millivolts.size());
      } else {
        std::lock_guard<std::mutex> guard(g_lock);
        g_status[i].state = "error";
        g_status[i].error = "adc";
        g_have[i] = false;
      }
      esp_task_wdt_reset();
      vTaskDelay(1);   // let the other tasks run between two inputs
    }
    if (!any) vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// ---- the continuous read (analog_stream.continuous) ----------------------------------------------------------------------------------
// The converter fills a buffer by itself at a fixed rate (DMA) while one input is read; the samples are converted to millivolts with the chip's own calibration and given, a window at a
// time, to core/adc_stream.hpp, which measures over whole cycles of the mains. It replaces the one-shot task above (the two cannot share the converter), and it is off by default.
// Nothing here has run on a board.
constexpr std::size_t kFrameResults = 256;   // conversions in one frame of the DMA

void stream_task(void*) {
  const adcstream::StreamSettings stream = g_settings.analog_stream;
  std::array<adc_cali_handle_t, board::kAnalogCount> calibration{};
  std::array<bool, board::kAnalogCount> ready{};
  std::array<analog::Energy, board::kAnalogCount> energy;
  std::array<std::uint64_t, board::kAnalogCount> last_ms{};
  for (std::size_t i = 0; i < board::kAnalogCount; ++i) {
    const config::AnalogSetting& input = g_settings.analogs[i];
    if (!input.enabled) continue;
    adc_cali_curve_fitting_config_t cali_config{};
    cali_config.unit_id = ADC_UNIT_1;
    cali_config.chan = static_cast<adc_channel_t>(board::adc1_channel(input.pin));
    cali_config.atten = ADC_ATTEN_DB_12;
    cali_config.bitwidth = ADC_BITWIDTH_12;
    std::lock_guard<std::mutex> guard(g_lock);
    if (adc_cali_create_scheme_curve_fitting(&cali_config, &calibration[i]) == ESP_OK) { ready[i] = true; g_status[i].state = "waiting"; }
    else { ESP_LOGE(kTag, "GPIO %d could not be calibrated", input.pin); g_status[i].state = "error"; g_status[i].error = "adc"; }
  }
  adcstream::WindowCollector collector(stream);
  std::vector<double> frame;
  frame.reserve(kFrameResults);
  static std::uint8_t bytes[kFrameResults * SOC_ADC_DIGI_RESULT_BYTES];
  esp_task_wdt_add(nullptr);   // as the one-shot task: a task that hangs here is restarted by the watchdog
  for (;;) {
    esp_task_wdt_reset();
    bool any = false;
    for (std::size_t i = 0; i < board::kAnalogCount; ++i) {
      const config::AnalogSetting& input = g_settings.analogs[i];
      if (!input.enabled || !ready[i]) continue;
      any = true;
      const adc_channel_t channel = static_cast<adc_channel_t>(board::adc1_channel(input.pin));
      collector.restart();
      bool started = false;
      {
        std::lock_guard<std::mutex> adc(armor::adc_lock());   // nobody else touches the converter during a window (see entropy.cpp)
        adc_continuous_handle_t handle = nullptr;
        adc_continuous_handle_cfg_t handle_config{};
        handle_config.max_store_buf_size = sizeof bytes * 4;
        handle_config.conv_frame_size = sizeof bytes;
        if (adc_continuous_new_handle(&handle_config, &handle) == ESP_OK) {
          adc_digi_pattern_config_t pattern{};
          pattern.atten = ADC_ATTEN_DB_12;
          pattern.channel = static_cast<std::uint8_t>(channel);
          pattern.unit = ADC_UNIT_1;
          pattern.bit_width = ADC_BITWIDTH_12;
          adc_continuous_config_t digital{};
          digital.pattern_num = 1;
          digital.adc_pattern = &pattern;
          digital.sample_freq_hz = static_cast<std::uint32_t>(stream.sample_rate_hz);
          digital.conv_mode = ADC_CONV_SINGLE_UNIT_1;
          digital.format = ADC_DIGI_OUTPUT_FORMAT_TYPE2;
          if (adc_continuous_config(handle, &digital) == ESP_OK && adc_continuous_start(handle) == ESP_OK) {
            started = true;
            const std::uint64_t deadline = static_cast<std::uint64_t>(esp_timer_get_time()) + static_cast<std::uint64_t>(stream.window_ms) * 3000ULL + 200000ULL;   // three times the window, and a little more
            while (!collector.full() && static_cast<std::uint64_t>(esp_timer_get_time()) < deadline) {
              std::uint32_t got = 0;
              const esp_err_t read = adc_continuous_read(handle, bytes, sizeof bytes, &got, 100);
              if (read == ESP_ERR_TIMEOUT) continue;
              if (read == ESP_ERR_INVALID_STATE) { collector.note_overrun(); break; }   // the buffer overflowed: some frames are lost, the window is not measured
              if (read != ESP_OK) break;
              frame.clear();
              for (std::uint32_t at = 0; at + SOC_ADC_DIGI_RESULT_BYTES <= got; at += SOC_ADC_DIGI_RESULT_BYTES) {
                const adc_digi_output_data_t* result = reinterpret_cast<const adc_digi_output_data_t*>(&bytes[at]);
                if (result->type2.channel != static_cast<std::uint32_t>(channel)) continue;
                int mv = 0;
                if (adc_cali_raw_to_voltage(calibration[i], static_cast<int>(result->type2.data), &mv) == ESP_OK) frame.push_back(static_cast<double>(mv));
              }
              collector.add(frame.data(), frame.size());
              esp_task_wdt_reset();
            }
            adc_continuous_stop(handle);
          }
          adc_continuous_deinit(handle);
        }
      }
      const std::uint64_t at = now_ms();
      const std::uint64_t elapsed = last_ms[i] == 0 ? 0 : at - last_ms[i];
      electrical::AnalogChannel made;
      adcstream::Measurement measured;
      if (started && collector.full() && !collector.broken()) measured = adcstream::measure(collector.samples(), stream);
      if (analog::make_channel_measured(input, measured, elapsed, energy[i], made)) {
        last_ms[i] = at;
        std::lock_guard<std::mutex> guard(g_lock);
        g_channels[i] = made;
        g_read_at_ms[i] = at;
        g_have[i] = true;
        g_status[i].state = "reading";
        g_status[i].error.clear();
        g_status[i].millivolts = measured.mean_mv;
        g_status[i].value = analog::quantity_from(input, measured.mean_mv, measured.rms_mv);
        g_status[i].samples = static_cast<std::uint32_t>(measured.samples_used);
      } else {
        std::lock_guard<std::mutex> guard(g_lock);
        g_status[i].state = "error";
        g_status[i].error = "adc";
        g_have[i] = false;
      }
      esp_task_wdt_reset();
      vTaskDelay(1);
    }
    if (!any) vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
}  // namespace

void start(const config::Settings& settings) {
  g_settings = settings;
  bool any = false;
  for (std::size_t i = 0; i < board::kAnalogCount; ++i) {
    g_status[i].enabled = settings.analogs[i].enabled;
    g_status[i].state = settings.analogs[i].enabled ? "waiting" : "disabled";
    any = any || settings.analogs[i].enabled;
  }
  // below the tasks that talk to the network and the relays; the stream (analog_stream.continuous, off by default) replaces the one-shot task: they cannot share the converter
  if (any) xTaskCreate(settings.analog_stream.continuous ? stream_task : analog_task, "analog", settings.analog_stream.continuous ? 8192 : 6144, nullptr, 2, nullptr);
}

std::vector<electrical::AnalogChannel> channels() {
  std::vector<electrical::AnalogChannel> out;
  const std::uint64_t now = now_ms();
  std::lock_guard<std::mutex> guard(g_lock);
  for (std::size_t i = 0; i < board::kAnalogCount; ++i) if (g_have[i] && now - g_read_at_ms[i] <= kFreshMs) out.push_back(g_channels[i]);
  return out;
}

std::array<InputStatus, board::kAnalogCount> status() { std::lock_guard<std::mutex> guard(g_lock); return g_status; }

}  // namespace armor::analoginputs
