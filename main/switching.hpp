// ARMOR-NODE-ELECTRICAL - the switching image: one transfer switch (two contactors, two auxiliary contacts) driven by the rules of core/switch_set.hpp.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// This exists only in an image built with CONFIG_ARMOR_SWITCHING (off by default; the images that are published never have it). In every other image the functions below are
// empty stand-ins: the node says `switching_enabled: false`, subscribes to nothing and drives no pin. Nothing here has run on a board.
#pragma once
#include <cstdint>
#include <functional>
#include <string>

#include "sdkconfig.h"

namespace armor::switching {

using Publish = std::function<void(const std::string& topic, const std::string& payload)>;

// What the state message of the node says about the switches: whether this firmware may switch at all and the `switches` array (empty text: none).
struct Report {
  bool enabled = false;
  std::string json;
};

#if CONFIG_ARMOR_SWITCHING

// Sets the pins to "open" and starts the task that reads the auxiliary contacts and drives the coils. `command_key` is the key of the node for the signed commands (empty: any command
// of the contract is taken). Listens to armor/electrical/<node>/command and answers on armor/electrical/<node>/result. Call it BEFORE mqtt_link::start() (the subscription is made
// when the broker connects).
void start(const std::string& node_id, const std::string& command_key, Publish publish);

// The node's work is off (paused or in standby by the server's order): every switch is opened and none closes until `hold` is false again.
void hold_open(bool hold);

Report report();

#else

inline void start(const std::string&, const std::string&, Publish) {}
inline void hold_open(bool) {}
inline Report report() { return {}; }

#endif

}  // namespace armor::switching
