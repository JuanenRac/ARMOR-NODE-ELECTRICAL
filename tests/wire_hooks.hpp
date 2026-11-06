// ARMOR-NODE-ELECTRICAL - the parsers this node has written by hand for the messages it receives, offered to tests/test_wire.cpp so that they are run against the oracle of ARMOR-COMMON.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include "../core/switch_set.hpp"

static std::vector<Hook> node_hooks() {
  return {
      {"electrical_command", "electrical::parse_command", [](const std::string& text) { armor::electrical::SwitchCommand command; return armor::electrical::parse_command(text, command); }},
  };
}
