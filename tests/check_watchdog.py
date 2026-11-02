#!/usr/bin/env python3
# ARMOR-NODE-ELECTRICAL - every task that keeps the node's job going is watched by the task watchdog: reads main/*.cpp (nothing is compiled) and fails when one of them does not
# register itself with esp_task_wdt_add(nullptr) or never feeds the watchdog inside its loop. A task that hangs would otherwise leave its readings, its relays or the Zigbee radio dead
# without anybody seeing it; registered, the node restarts instead.
# Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# file -> the functions that are the body of a watched task (the meters, the one-shot and the continuous read of the analog inputs, the relays, the Zigbee bridge)
WATCHED = {
    "main/electrical_manager.cpp": ["bus_task"],
    "main/analog_inputs.cpp": ["analog_task", "stream_task"],
    "main/relay_outputs.cpp": ["watch_task"],
    "main/zigbee_bridge.cpp": ["bridge_task"],
}


def body_of(text: str, name: str) -> str | None:
    match = re.search(r"\bvoid\s+" + re.escape(name) + r"\s*\(void\*[^)]*\)\s*\{", text)
    if not match:
        return None
    depth, i = 1, match.end()
    while i < len(text) and depth:
        depth += {"{": 1, "}": -1}.get(text[i], 0)
        i += 1
    return text[match.end():i]


def main() -> int:
    problems: list[str] = []
    for relative, names in WATCHED.items():
        path = ROOT / relative
        if not path.is_file():
            problems.append(f"{relative}: missing")
            continue
        text = path.read_text(encoding="utf-8")
        for name in names:
            body = body_of(text, name)
            if body is None:
                problems.append(f"{relative}: the task body {name}() was not found (rename it in tests/check_watchdog.py too)")
                continue
            if "esp_task_wdt_add(nullptr)" not in body:
                problems.append(f"{relative}: {name}() never registers with the task watchdog (esp_task_wdt_add(nullptr))")
            if "esp_task_wdt_reset()" not in body:
                problems.append(f"{relative}: {name}() never feeds the task watchdog (esp_task_wdt_reset())")
    for problem in problems:
        print("FAIL", problem)
    print(f"{sum(len(n) for n in WATCHED.values())} watched tasks, {len(problems)} problems")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
