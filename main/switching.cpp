// ARMOR-NODE-ELECTRICAL - the switching image (see switching.hpp). Nothing here has run on a board.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// What is decided is in core/switch_set.hpp and core/interlock.hpp (tested on a computer, 135,000 random steps with welded and slow contactors): who may close, in which order,
// after how long and what a fault does. This file only reads the two auxiliary contacts, puts the two coil levels on their pins and carries the command and its answer to the broker.
// The level of a coil pin is set BEFORE the pin becomes an output, so a coil never sees a pulse while the node starts, and it is written again at every turn, so a pin that was disturbed
// is put back. Before the node runs, and while it restarts, a pin floats: the board must pull both coil drivers to their open level with a resistor of its own.
#include "switching.hpp"

#if CONFIG_ARMOR_SWITCHING

#include <memory>
#include <mutex>
extern "C" {
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
}
#include "core/switch_set.hpp"
#include "entropy.hpp"
#include "mqtt_link.hpp"

namespace armor::switching {
namespace {
constexpr char kTag[] = "armor-switching";
constexpr char kSwitchId[] = "transfer";
constexpr gpio_num_t kCoilA = static_cast<gpio_num_t>(CONFIG_ARMOR_SWITCH_COIL_A_GPIO);
constexpr gpio_num_t kCoilB = static_cast<gpio_num_t>(CONFIG_ARMOR_SWITCH_COIL_B_GPIO);
constexpr gpio_num_t kAuxA = static_cast<gpio_num_t>(CONFIG_ARMOR_SWITCH_AUX_A_GPIO);
constexpr gpio_num_t kAuxB = static_cast<gpio_num_t>(CONFIG_ARMOR_SWITCH_AUX_B_GPIO);
#if CONFIG_ARMOR_SWITCH_COIL_ACTIVE_LOW
constexpr int kCoilOn = 0;
#else
constexpr int kCoilOn = 1;
#endif
#if CONFIG_ARMOR_SWITCH_AUX_ACTIVE_LOW
constexpr int kAuxClosed = 0;
#else
constexpr int kAuxClosed = 1;
#endif

std::mutex g_lock;
std::unique_ptr<electrical::SwitchSet> g_set;
std::string g_node_id;
Publish g_publish;
bool g_hold = false;

std::uint64_t now_ms() { return static_cast<std::uint64_t>(esp_timer_get_time()) / 1000ULL; }

void drive(bool a, bool b) {
  gpio_set_level(kCoilA, a ? kCoilOn : !kCoilOn);
  gpio_set_level(kCoilB, b ? kCoilOn : !kCoilOn);
}

void open_pins() {
  gpio_set_level(kCoilA, !kCoilOn);   // the level first, then the direction: no pulse of the wrong level
  gpio_set_level(kCoilB, !kCoilOn);
  gpio_config_t out{};
  out.pin_bit_mask = (1ULL << kCoilA) | (1ULL << kCoilB);
  out.mode = GPIO_MODE_OUTPUT;
  gpio_config(&out);
  gpio_set_level(kCoilA, !kCoilOn);
  gpio_set_level(kCoilB, !kCoilOn);
  gpio_config_t in{};
  in.pin_bit_mask = (1ULL << kAuxA) | (1ULL << kAuxB);
  in.mode = GPIO_MODE_INPUT;
  in.pull_up_en = kAuxClosed == 0 ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
  in.pull_down_en = kAuxClosed == 1 ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
  gpio_config(&in);
}

void switching_task(void*) {
  esp_task_wdt_add(nullptr);   // a task that hangs would leave a coil as it was: the watchdog restarts the board, and the pins fall to open with it
  for (;;) {
    esp_task_wdt_reset();
    bool a = false, b = false;
    {
      std::lock_guard<std::mutex> guard(g_lock);
      const std::uint64_t now = now_ms();
      g_set->set_feedback(kSwitchId, electrical::Feedback{gpio_get_level(kAuxA) == kAuxClosed, gpio_get_level(kAuxB) == kAuxClosed});
      if (g_hold) g_set->open_all(now);
      g_set->tick(now);
      const electrical::Coils coils = g_set->coils(kSwitchId);
      a = coils.a && !g_hold;
      b = coils.b && !g_hold;
    }
    drive(a && !b, b && !a);   // never both, whatever the controller said
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void on_command(const std::string&, const std::string& payload) {
  std::string answer;
  {
    std::lock_guard<std::mutex> guard(g_lock);
    if (!g_set) return;
    const std::int64_t wall = mqtt_link::clock_is_set() ? static_cast<std::int64_t>(mqtt_link::wall_clock_ms()) : 0;
    answer = g_set->handle(payload, now_ms(), wall);
  }
  if (!answer.empty() && g_publish) g_publish(electrical::result_topic(g_node_id), answer);
}
}  // namespace

void start(const std::string& node_id, const std::string& command_key, Publish publish) {
  open_pins();
  g_node_id = node_id;
  g_publish = std::move(publish);
  auto set = std::make_unique<electrical::SwitchSet>(node_id, [](std::uint8_t* out, std::size_t length) {
    armor::random_bytes(out, length);
    return true;
  });
  electrical::SwitchConfig config;
  config.id = kSwitchId;
  config.label = "Transfer";
  config.transfer.allowed = true;
  config.transfer.dead_time_ms = CONFIG_ARMOR_SWITCH_DEAD_TIME_MS;
  config.transfer.confirm_timeout_ms = CONFIG_ARMOR_SWITCH_CONFIRM_TIMEOUT_MS;
  config.transfer.arm_window_ms = CONFIG_ARMOR_SWITCH_ARM_WINDOW_MS;
  if (!set->add(config)) {
    ESP_LOGE(kTag, "the switch could not be added (node id %s): the node does not switch", node_id.c_str());
    return;
  }
  if (!command_key.empty()) set->set_command_key(command_key);
  {
    std::lock_guard<std::mutex> guard(g_lock);
    g_set = std::move(set);
  }
  const std::string topic = electrical::command_topic(node_id);
  if (topic.empty()) {
    ESP_LOGE(kTag, "the node id has no command topic: the node does not switch");
    return;
  }
  mqtt_link::subscribe(topic, on_command);
  xTaskCreate(switching_task, "switching", 4096, nullptr, 6, nullptr);
  ESP_LOGW(kTag, "SWITCHING IMAGE: this firmware can close contactors on %s (coils on GPIO %d and %d, contacts on GPIO %d and %d). Not for a published release.", topic.c_str(),
           static_cast<int>(kCoilA), static_cast<int>(kCoilB), static_cast<int>(kAuxA), static_cast<int>(kAuxB));
}

void hold_open(bool hold) {
  std::lock_guard<std::mutex> guard(g_lock);
  g_hold = hold;
}

Report report() {
  std::lock_guard<std::mutex> guard(g_lock);
  if (!g_set) return {};
  return Report{g_set->switching_enabled(), g_set->switches_json(now_ms())};
}

}  // namespace armor::switching

#endif  // CONFIG_ARMOR_SWITCHING
