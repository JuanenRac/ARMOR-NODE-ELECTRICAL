<p align="center">
  <img src="images/ARMOR_BANNER.svg" alt="ARMOR-NODE-ELECTRICAL banner" width="100%">
</p>

# ⚡ ARMOR-NODE-ELECTRICAL

<p align="center">
  🇺🇸 <b>English</b> |
  <a href="README_spa.md">🇪🇸 Español</a> |
  <a href="README_fra.md">🇫🇷 Français</a> |
  <a href="README_ita.md">🇮🇹 Italiano</a> |
  <a href="README_deu.md">🇩🇪 Deutsch</a> |
  <a href="README_zho.md">🇨🇳 简体中文</a> |
  <a href="README_jpn.md">🇯🇵 日本語</a> |
  <a href="README_por.md">🇵🇹 Português</a>
</p>

### Electrical node: reads the voltages, currents, power and energy of the house's AC and DC network and holds the rules for switching it (host-tested core and a firmware that builds; it has never run on a board)

<p align="center">
  <img src="https://img.shields.io/badge/License-GPL%203.0--or--later-blue.svg" alt="GPL 3.0 or later">
  <img src="https://img.shields.io/badge/Language-C%2B%2B17-00599c.svg" alt="Language">
  <img src="https://img.shields.io/badge/Meters-PZEM--004T%20%2F%20017-ffb020.svg" alt="Meters">
  <img src="https://img.shields.io/badge/Checks-376%2C195-2ea44f.svg" alt="Checks">
  <img src="https://img.shields.io/badge/Tests-382264%20checks-2ea44f.svg" alt="Tests">
  <img src="https://img.shields.io/badge/Version-0.5.2-0f6cbd.svg" alt="Version">
  <img src="https://img.shields.io/badge/Maturity-functional-00E5FF.svg" alt="Maturity">
</p>

---

**Honesty check - what runs today:** The core (the frames of the PZEM-004T v3 and PZEM-017 meters, the bus that asks them, the message of the node, the analogue inputs and the rules for switching: 382,264 checks in 19 test programs, 375,036 of them on the switching rules) is tested on a computer with stand-in meters and contactors, and the messages it makes are accepted by ARMOR-COMMON. The firmware of the node (the settings, the serial line, the Meters and Readings pages of its panel, Bluetooth, signed updates) builds in the ESP-IDF 5.5.5 container for both boards. **It has never run on a board, nothing has been connected to a meter or a contactor, and nothing here switches anything.**

---

## 📋 At a glance

| | |
|---|---|
| Version | `0.5.2` |
| Maturity | Functional |
| Role | Electrical field node: meters, inputs and switching (family: Field Nodes, parent: ARMOR-COMMON) |
| Stack | C++17, ESP-IDF 5.5.5, ESP32-S3 |
| Tests | 382264 checks in 19 host programs and 5 Python checks (`ctest`) |
| Equipment | PZEM-004T v3 and PZEM-017 meters (up to 16), analogue inputs, relays; switching off by default |
| Runs on | An ESP32-S3 board (Ethernet or Wi-Fi); host tests on any computer |
| On real hardware | None yet: no board built and no meter or contactor connected; nothing switches anything |
| License | GPL-3.0-or-later |

**Jump to:** [Overview](#-overview) · [Architecture](#-architecture) · [Security](#-security--safety) · [Interfaces](#-interfaces) · [Configuration](#-configuration) · [Quick start](#-quick-start) · [Testing](#-testing) · [Troubleshooting](#-troubleshooting) · [Structure](#-repository-structure) · [Related](#-related-projects)

## 🚦 Project status

Each line is one part of this repository and the light says how far it is today. It is written from what the repository can show (tests, builds, runs), not from what is planned.

🟢 Done and verified · 🟡 Works, with known limits · 🟠 Partly done or not verified yet · 🔴 Not done or not working

| Area | State | What it means here |
|---|---|---|
| Software | 🟢 | The firmware builds in the ESP-IDF 5.5.5 container for both boards (Wi-Fi and Ethernet). |
| Tests | 🟢 | 382,264 checks in 19 host programs pass (`ctest`), 375,036 of them on the switching rules; the meters and contactors are stand-ins. |
| Protocols and data | 🟡 | The PZEM-004T v3 and PZEM-017 frames and readings are checked against the documented frames and recorded samples; no meter has answered yet. |
| Interface | 🟡 | The Meters and Readings pages of the web panel are exercised in a browser against a stand-in node, not on a board. |
| Hardware | 🔴 | No board built and no meter or contactor connected: nothing has run on real equipment yet. |
| Security | 🟡 | The set-up code, the login with a limit on wrong passwords, the Bluetooth PIN and the signed updates are tested on the host; there has been no independent review. |
| Updates | 🟠 | The hash and Ed25519 signature checks and the rollback logic pass on the host; an update over the air has never been run on a board. |
| Writing to equipment | 🟠 | The rules for switching contactors are tested only against stand-in contactors; nothing here has switched anything real. |
| Integration | 🟠 | The messages of the node are validated against the shared schemas and conformance vectors of ARMOR-COMMON. Its MQTT client is played against a real Mosquitto in the tests; no board has sent them to a server yet. |
| Documentation | 🟢 | Eight languages, the firmware guide, the wiring notes and the changelog are up to date. |

## 🎯 Overview

* **Reading the meters:** the Modbus RTU frames of the Peacefair **PZEM-004T v3** (AC) and **PZEM-017** (DC): the CRC, the request that reads the registers and the decoding of the reply (voltage, current, power, energy, frequency, power factor, alarm). Only the read request is built: the ones that write to a meter (its address, its thresholds, the reset of its energy) are not built anywhere. A reply that does not fit, or holds a value no real meter could give, is refused.
* **The bus:** a line with several meters is asked one at a time, with a timeout; the last good reading of each is kept and a meter that goes silent stops being published after ten seconds.
* **The message** `armor/electrical/<node>/state`: one entry per channel (a circuit, a line, the grid input, a DC bus) with AC or DC, voltage, current, power, energy, frequency, power factor, the state of a switch as the node sees it and an alarm; it is in the shared contract and carries a state, never a command.
* **The rules for switching,** apart from any hardware: a controller for the transfer between two sources that never commands both, needs the switching allowed, the node armed just before, both contactors confirmed open for the whole dead time, and turns a contactor that does not show what it was told into a fault that stays until it is acknowledged. **Switching is off by default and nothing drives any hardware.**
* **The commands to a switch, not activated:** the state message can carry the state of the node's switches, and the shared contract has a command (`arm`, then `close_a` or `close_b` with a one-time token, `open`, `acknowledge`) and the node's answer; the core reads and answers them with the rules above, tested against the shared vectors and with stand-in contactors. No firmware image includes it, so nothing can drive a contactor, and ARMOR-SERVER refuses to send a command unless it was turned on (`ARMOR_ELECTRICAL_SWITCHING=1`) and the broker's ACL allows it.
* **Where it shows:** the *Electrical Designer* of ARMOR-STUDIO draws the house's network and shows on each element what its channel measures; ARMOR-SERVER keeps the readings, their history and the sums.
* **The firmware of the node** (ESP32-S3-WROOM-1 N16R8 on Wi-Fi, or the Waveshare ESP32-S3-ETH on its cable): the settings, one serial line for up to sixteen meters and the web panel of the other nodes (set-up, users, Wi-Fi, broker, update over the air, HTTPS) with its own Meters and Readings pages, in eight languages. It only reads: the rules for switching are not linked into it. See [the firmware](docs/NODE_FIRMWARE.md).
* **Configuration from a phone over Bluetooth,** the same channel as the radar node's: the ARMOR app finds the node as `ARMOR-XXXXXX` and sets its name, Wi-Fi, address, broker and Bluetooth mode with the panel's users and set-up code ([the protocol](docs/BLE_PROVISIONING.md)). It listens only while the node has no user, unless told otherwise. The radio side has never run on a board.
* **Signed over-the-air updates:** the node installs from GitHub only an image whose SHA-256 matches and that carries a valid Ed25519 signature of the maintainer (the public key is in the firmware, the private one never leaves the maintainer); a file uploaded by hand by an administrator in the panel is still accepted. The pages and messages of the panel are in eight languages.
* **Bluetooth pairing with a PIN:** the six-digit PIN is derived from the set-up code, printed on the USB console and shown to an administrator in the panel; the phone has to enter it before it may write the node's settings.
* **Not yet:** the firmware running on a board, the hardware, any measurement of a real installation and any switching.
* **Only the house may use the panel:** the panel answers the private network by default (or the networks its owner lists, such as a VPN), refuses a list that would lock out whoever saves it, counts wrong passwords for each person as well as each address, and works behind the server's proxy so that it can be reached from outside the house without publishing the node.
* **Estimates say so:** a channel made from a current input carries `power_estimated`, so Studio and the server show its power as an estimate (apparent power from a current and a nominal voltage) and mark a grid figure that includes one.
* **Optional encrypted link to the broker (TLS):** off by default, so the node talks plain MQTT as before. A switch and a box for the certificate of the installation's own authority (PEM, at most 2 KB) in the Broker page make it connect with `mqtts://`, trust that authority only and check that the broker's certificate was made for the address it reached; anything that is not one PEM certificate is refused. Covered by host tests; the handshake has not been run on a board yet.
* **Health report:** about once a minute the node publishes how long it has been up and why it last restarted, its free memory, its link and Wi-Fi signal, how many times it connected to the broker and the last error it logged (`armor/health/<node>`, the `node_health` contract of ARMOR-COMMON). The code that builds it is shared by every node and tested on a computer; it has not run on a board.
* **Control from the server:** Studio can restart the node cleanly, pause it for up to an hour (it resumes by itself), put its work in standby (no end, kept across restarts, the node stays connected and keeps answering), make it show itself, ask for a diagnostics snapshot and mark it as in maintenance (`armor/control/<node>/...`, the `node_control` contract of ARMOR-COMMON). Every order takes two steps with a one-time token that belongs to that operation, only the server may write them on the broker, and there is no factory reset, no erase and no deep sleep. While the node is paused or in standby every relay is opened and none moves, whoever asks (the panel, the broker or a central); the meters keep being read. The code is shared by every node and tested on a computer; it has not run on a board.
* **Readings that say how fresh they are:** each meter channel carries the age of its reading and how many requests to the meter failed, so the server tells a real 0 W from a meter that stopped answering; and a hung meters task restarts the board through the watchdog.
* **Diagnostics without a cable:** after a start that was not clean (a crash, a watchdog, a sag of the supply, a failed allocation) the health report says why for ten minutes; a crash leaves its core dump to look at, download or erase from the panel; the stored settings are copied before every firmware install and recovered if an update damages them; and an administrator can ask the node, through the server, for a diagnostic package or its log with a one-time token.
* **Relay interlock:** a relay can name another that must never be closed at the same time (the two sources of a load, the two directions of a motor); a request to close one while the other is closed is refused from the panel, the broker and the central alike, and the node never opens the partner by itself. Tested on a computer; not run on a board.
* **Read-only reading of the meters' own settings:** after the first good reading and every `bus.check_settings_s` (600 s, 0 = never) the node reads the PZEM holding registers (004T v3: power threshold and address; 017: voltage thresholds, address and shunt) in the same turn as the measurements, and warns when the stored address is not the one it uses; nothing is written to the meter.
* **Optional sealing of stored secrets:** off by default (`security.seal_secrets`, or the server's `nodes.nvs_encryption`). When on, the passwords, tokens, keys, PINs and password hashes the node stores are kept sealed with ChaCha20-Poly1305 and everything else stays readable; turning it on or off rewrites them with nothing lost, even if the node stops half-way. No eFuse is burned and the flash is not encrypted: the key lives in a partition of the same chip, so it does **not** protect against someone who holds the board, and a node installed before that partition existed keeps its secrets as they were until it is flashed with the new partition table.

## 🧩 Role in A.R.M.O.R.

ARMOR-NODE-ELECTRICAL measures the house's AC and DC network through its meters and holds the rules for switching between sources. It publishes the readings for [ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER), which keeps their history and the energy of each day and raises the alarms; the *Electrical* menu and the *Electrical Designer* of [ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO) and the electrical screen of [ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL) show it. Switching is designed (arm, a one-time token, the interlock, the confirmation of both contactors) but not activated.

| Name | How | What for |
|---|---|---|
| ARMOR-SERVER | MQTT `armor/electrical/<id>/{state,command,result}` | Receives the channels and orders a transfer (only when switching is on in three places) |
| ARMOR-STUDIO | Electrical menu, Electrical Designer | Shows the channels and the alarms and opens the node's panel |
| ARMOR-ANDROID-CONTROL | Bluetooth | Sets the node up in the field and shows the channels |
| ARMOR-COMMON | electrical contract | The messages are checked against its schemas; the common firmware is copied from it |

## 🏛️ Architecture

<p align="center">
  <img src="images/architecture.svg" alt="ARMOR-NODE-ELECTRICAL architecture: who calls it, what it holds and what it depends on" width="100%">
</p>

## 🖼️ Screenshots

Captured at 1920×1080 on a test bench, with sample data and no real installation behind them.

<table>
<tr><td width="50%" valign="top"><img src="images/screenshots/overview.png" alt="Overview: the state of the node at a glance - network, broker, firmware and what it reports." width="100%"><br><sub>Overview: the state of the node at a glance - network, broker, firmware and what it reports.</sub></td><td width="50%" valign="top"><img src="images/screenshots/meters.png" alt="Meters: the energy meters and analog channels the node reads, each with its name and scale." width="100%"><br><sub>Meters: the energy meters and analog channels the node reads, each with its name and scale.</sub></td></tr>
<tr><td width="50%" valign="top"><img src="images/screenshots/readings.png" alt="Readings: what the node is reading right now, with the age of each value." width="100%"><br><sub>Readings: what the node is reading right now, with the age of each value.</sub></td><td width="50%" valign="top"><img src="images/screenshots/zigbee.png" alt="Zigbee radio: the serial bridge to the coordinator module, which client may use it, and the state of the bridge." width="100%"><br><sub>Zigbee radio: the serial bridge to the coordinator module, which client may use it, and the state of the bridge.</sub></td></tr>
</table>

## 🔒 Security & Safety

| Threat | Defence |
|---|---|
| Passwords in clear or compiled in | PBKDF2-HMAC-SHA256 with 10 000 rounds; no password in the firmware (a node with no user enters set-up); at most 4 users (admin or viewer) and 8 to 64 characters; at most 6 sessions, 30 minutes idle |
| Taking a freshly flashed node | Set-up needs a start-up code: the one of the build (`CONFIG_ARMOR_SETUP_CODE`) or a random one kept in flash and shown on the USB console; with a fleet secret it is `HMAC-SHA256(secret, MAC)`; it is also the key of the set-up Wi-Fi (WPA2, never bridged) |
| The panel reached from the internet | It answers the private networks of a house by default (10/8, 172.16/12, 192.168/16, 169.254/16, 127/8 and 100.64/10 where some VPNs live); the setting `web.admin_sources` lists other IPv4 addresses and networks (8 entries, 120 characters); anything outside is refused except the public identity `GET /api/v1/session`; a list that would lock out the person saving it is rejected |
| The panel in clear over HTTP | HTTPS on 443 with a certificate the node makes itself (self-signed, in flash, SHA-256 fingerprint shown to compare); the panel can be http, https or both; the session cookie is always HttpOnly and SameSite=Strict (and Secure over TLS) |
| A fake firmware from GitHub | An image is installed only with a matching SHA-256 and a valid Ed25519 signature made with the maintainer's key (`no_checksum`, `no_signature`, `bad_signature`, `checksum_mismatch`, `wrong_firmware`, `not_newer`, `version_mismatch` refuse it); a manual upload by an administrator is not checked: an explicit limit |
| Hijacking the Bluetooth set-up | A 6-digit PIN derived from the set-up code, shown on the USB console and to an administrator; the phone must type it before writing settings; the set-up window lasts `CONFIG_ARMOR_BLE_SETUP_MINUTES` (10 by default, 1 to 120) |
| A remote order that does harm | Two steps with a single-use token tied to that operation; only the server may write those topics to the broker; a pause lasts one hour at most; there is no factory reset, no erase and no deep sleep by remote order |
| Stored secrets read by someone who has the board | Optional (off by default, switch `security.seal_secrets` or the server's `nodes.nvs_encryption`): ChaCha20-Poly1305 for passwords, tokens, keys, PINs and hashes. It does not encrypt the flash or burn an eFuse: the key is in a partition of the same chip, so it does not protect against whoever holds the board |
| The broker link in clear | Optional TLS (`mqtts://`) with the owner's own authority (PEM up to 2 KB) and a check of the name; off by default. The handshake has only run in host tests, not on a board |
| Dangerous mains or DC on the board | The node only reads the PZEM modules (closed modules, isolated serial line) and never carries mains on its board; whatever switches is a separate stage tested on the bench with a lamp and an isolating transformer |
| Two sources closed at once | The transfer interlock never orders both: it needs switching allowed, the node armed just before and both contactors confirmed open for the whole dead time (`dead_time_ms` 2000 by default); a contactor that does not show what was ordered is a fault that stays until acknowledged (`kDidNotOpen`, `kDidNotClose`); a mechanical interlock between the contactors is still required |
| A repeated order, or switching nobody asked for | `arm` gives a single-use 64-bit token that `close_a` or `close_b` must carry, inside `arm_window_ms`; orders are never retained; at most 4 switches; switching is OFF unless the firmware allows it (`switching_enabled`), the server runs with `ARMOR_ELECTRICAL_SWITCHING=1` and the broker's access list is open for that node |
| A fault, a lost feedback or switching off | Each controller answers with ALL coils open (fail open; whether open is safe for the load is for the installation to decide); the state in the message is what the auxiliary contact shows, not what was ordered; relay pairs are interlocked and the node never opens the partner by itself |
| A stale reading shown as real | A mute meter stops being published after 10 s; each channel carries the age of its reading and the number of failed requests; a stuck meter task restarts the board by watchdog; every `bus.check_settings_s` (600 s, 0 = never) the meters' settings are read (read only) and a changed address is reported |

Mains (230 V AC) and the DC of a battery bank can kill and start fires. [docs/SAFETY.md](docs/SAFETY.md) says what the design assumes; it is not a design of an installation and it does not replace the regulations of the country or a qualified installer. Nothing here switches anything today.

## 🔌 Interfaces

| Part | What it sets |
|---|---|
| MQTT | `armor/electrical/<id>/state` (one channel per input: AC or DC, voltage, current, power, energy, frequency, power factor, switch state, alarm; ARMOR-COMMON contract), `armor/electrical/<id>/command` and `/result`, `armor/device/<id>/<relay>/{state,set}`, `armor/health/<id>`, `armor/control/<id>/...` |
| Commands | `arm`, `close_a`, `close_b` (with the token), `open` and `acknowledge` (clears a fault) |
| Meters | Peacefair PZEM-004T v3 (AC) and PZEM-017 (DC) by Modbus RTU, up to 16 on one serial line; only the read request is ever built (the one that writes address or thresholds does not exist); analogue inputs, contacts and relays on the base board; a Zigbee bridge ([docs/ZIGBEE.md](docs/ZIGBEE.md)) |

## ⚙️ Configuration

| Variable | Default | What it sets |
|---|---|---|
| `bus.check_settings_s` | 600 | Seconds between the read-only checks of the meters' settings (0 = never) |
| `dead_time_ms`, `confirm_timeout_ms` | 2000 | The dead time of a transfer and how long a contactor has to show what was ordered |
| `switching_enabled` | off | Switching is OFF unless this setting, the server (`ARMOR_ELECTRICAL_SWITCHING=1`) and the broker's access list all allow it |
| `CONFIG_ARMOR_NODE_ID`, `_SETUP_CODE`, `_FLEET_SECRET`, `_MQTT_*`, `_HEARTBEAT_S`, `_BLE_SETUP_MINUTES`, `_BOOT_FACTORY_RESET` | - | The settings every node shares: the name (empty gives `armor-` and the last bytes of the MAC), the set-up code (empty: random), the fleet secret (16+ characters, never in the repository), the first broker settings, the heartbeat (2 to 300 s), the minutes of Bluetooth set-up (1 to 120) and the reset with BOOT (8 s in the first 30 s) |

## 🚀 Quick start

**Requirements:** a C++17 compiler for the host tests; the ESP-IDF 5.5.5 container to build the image; an ESP32-S3 board (Ethernet or Wi-Fi).

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host   # host tests, -Werror
node tools/panel_mock.mjs --user admin:adminpass123   # the panel without a board
tools/build_node.sh generic s3-wifi                  # the image in the ESP-IDF container (or s3-eth)
```

See the [firmware guide](docs/NODE_FIRMWARE.md) and the [Bluetooth channel](docs/BLE_PROVISIONING.md).

See the [design](docs/DESIGN.md), the [safety notes](docs/SAFETY.md), the [rules for switching](docs/SWITCHING.md), the [protocols](docs/PROTOCOLS.md) and the [messages](docs/ELECTRICAL_MESSAGES.md).

## 🧪 Testing

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host
python -I tools/armor_ci_validate.py
```

19 host test programs and 5 Python checks run under `ctest`; 382264 checks counted by the host programs, built with `-Werror`.

Python checks: the panel texts in eight languages, the tasks, the save paths, the Bluetooth session under its lock and the routes the panel calls (and, in some nodes, the HTTP codes or the glyphs of the fonts)

The host tests need any C++17 compiler (Linux, WSL, MSYS2); the web panel is also exercised in a real browser against a stand-in node (`tools/panel_mock.mjs`, `tools/panel_browser_test.mjs`).

Most of the checks are exhaustive sweeps of the switching rules (`test_switch_set` and `test_interlock`: 375,036); the meters and contactors are stand-ins. `test_clock` prints only `ok`.

## 🏗️ Build & Release

`tools/build_node.sh` builds the image in the ESP-IDF 5.5.5 container and the release carries the image, its SHA-256 and its Ed25519 signature, one per board; the image is written by USB-C the first time and updated from the panel afterwards.

## 🩺 Troubleshooting

| You see | Because | Do |
|---|---|---|
| You lost the password or the node no longer joins the network | A node with set-up and no address after 90 s opens its own Wi-Fi `ARMOR-SETUP-xxxxxx` (open `http://192.168.4.1/`) | To start again hold BOOT for 8 seconds in the first 30 seconds after power-up (erases settings and users), or use *Factory reset* in the panel |
| An update from GitHub is refused | The release has no checksum or signature, they do not match (`e_no_checksum`, `e_no_signature`, `e_checksum_mismatch`, `e_bad_signature`), has no image for this board (`e_no_asset_for_board`) or is for another project (`e_wrong_firmware`) | Nothing changed on the node; try again or let an administrator upload the file by hand |
| A switch shows a fault | The contactor did not show open or closed as ordered (`kDidNotOpen`, `kDidNotClose`); it stays until acknowledged | Check the wiring of the auxiliary contact, then send `acknowledge` |
| A command is refused | Switching is off (`switching_enabled` false), the token is invalid or expired, or the node was not armed just before | Arm again and send the order with the new token |

## 📂 Repository Structure

<p align="center">
  <img src="images/repository-structure.svg" alt="Repository structure of ARMOR-NODE-ELECTRICAL" width="100%">
</p>

<details>
<summary>Text version of the tree</summary>

```text
ARMOR-NODE-ELECTRICAL/
├── main/                                the firmware: an ESP-IDF component
├── core/                                the logic with no ESP-IDF, tested on a computer
├── panel/                               the web panel embedded in the firmware
├── tests/                               host tests and Python checks
├── docs/                                12 guides: safety, switching, messages, hardware
├── tools/                               build, pack and check tools
├── .github/                             CI workflow, issue and pull-request templates
├── images/                              banner, icon and the two pictures of this README
├── build.sh build.bat                   build through the shared project workflow
├── test.sh test.bat                     run the tests through the shared workflow
├── run.sh run.bat                       start through the shared project workflow
├── armor.project.json                   manifest: version, role, maturity, build command
├── CHANGELOG.md                         what changed in each version
├── SECURITY.md SUPPORT.md LICENSE       security policy, support and license
├── CONTRIBUTING.md CODE_OF_CONDUCT.md   how to contribute and the code of conduct
├── CMakeLists.txt                       the ESP-IDF project
├── partitions.csv                       the flash table
├── sdkconfig.defaults                   the firmware settings
├── sdkconfig.board.s3-eth               the settings of one board
├── sdkconfig.board.s3-wifi              the settings of one board
├── secrets/                             example of the node file (the real ones are not versioned)
├── build-test.sh build-test.bat         run the tests through the shared workflow
└── .gitattributes .gitignore            Git settings
```

</details>

## 🗺️ Roadmap & Known limits

**Known limits today**

* The PZEM-004T v3 and PZEM-017 frames and readings are checked against the documented frames and recorded samples; no meter has answered yet.
* The Meters and Readings pages of the web panel are exercised in a browser against a stand-in node, not on a board.
* No board built and no meter or contactor connected: nothing has run on real equipment yet.
* The set-up code, the login with a limit on wrong passwords, the Bluetooth PIN and the signed updates are tested on the host; there has been no independent review.
* The hash and Ed25519 signature checks and the rollback logic pass on the host; an update over the air has never been run on a board.
* The rules for switching contactors are tested only against stand-in contactors; nothing here has switched anything real.

**Next**

* Try the meters, the contactors and the switching rules on a bench with a lamp and an isolating transformer, then a real board with the update over the air.

## 🌍 Languages

README and texts exist in English, Spanish, French, Italian, German, Chinese, Japanese and European Portuguese.

## 🔗 Related Projects

**A.R.M.O.R.** (Autonomous Radar & Multimodal Observation Range) is a perimeter-security system made of independent repositories. Each one has its own version, its own tests and its own README; this is the family:

* **[ARMOR-COMMON](https://github.com/JuanenRac/ARMOR-COMMON)** - Message contracts, validators, conformance vectors and generated types
* **[ARMOR-NODE-RADAR](https://github.com/JuanenRac/ARMOR-NODE-RADAR)** - Field-node firmware for ESP32-S3 with three radars and its own web panel
* **[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR)** - Solar inverter and battery protocols and the messages of a gateway node
* **ARMOR-NODE-ELECTRICAL** (this repository) - Electrical node: meters, the message of the network's readings and the rules for switching
* **[ARMOR-NODE-ALARM](https://github.com/JuanenRac/ARMOR-NODE-ALARM)** - Alarm node and panel: zones, arming, delays, siren and PIN, with the server or without it
* **[ARMOR-NODE-HMI](https://github.com/JuanenRac/ARMOR-NODE-HMI)** - Touch panel: the state of the system on a wall screen, arming and acknowledging, and the home of the voice assistant
* **[ARMOR-NETWORK](https://github.com/JuanenRac/ARMOR-NETWORK)** - The local network: its devices, the internet and what changes
* **[ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER)** - Central coordinator: telemetry, alarms, devices, solar readings and cameras
* **[ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO)** - Web console: cameras, radar, alarms, solar energy and the 2D/3D site designer
* **[ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL)** - Android operator client with a live 2D/3D radar
* **[ARMOR-IOS-CONTROL](https://github.com/JuanenRac/ARMOR-IOS-CONTROL)** - iPhone operator client
* **[ARMOR-SERVER-AI](https://github.com/JuanenRac/ARMOR-SERVER-AI)** - Visual inference policy that explains its decisions and never actuates
* **[ARMOR-VISION](https://github.com/JuanenRac/ARMOR-VISION)** - Camera zones, masks and detection rules for the Jetson Orin NX
* **[ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)** - Offline voice intents with a confirmation that cannot be forged
* **[ARMOR-HARDWARE](https://github.com/JuanenRac/ARMOR-HARDWARE)** - Enclosures, electronics and the bench acceptance matrix
* **[ARMOR-DEVOPS](https://github.com/JuanenRac/ARMOR-DEVOPS)** - Deployment, the CM5 test bench, backup and TLS
* **[ARMOR-SIMULATOR](https://github.com/JuanenRac/ARMOR-SIMULATOR)** - Offline telemetry simulator with repeatable faults
* **[ARMOR-UPDATER](https://github.com/JuanenRac/ARMOR-UPDATER)** - Detects, installs and updates the ecosystem's own repositories
* **[ARMOR-DOCS](https://github.com/JuanenRac/ARMOR-DOCS)** - Architecture, security baseline and the capability matrix

## 📚 Documentation

* [Safety](docs/SAFETY.md) and [switching](docs/SWITCHING.md): what the design assumes and how a transfer goes
* [Messages](docs/ELECTRICAL_MESSAGES.md), [protocols](docs/PROTOCOLS.md) and [Zigbee](docs/ZIGBEE.md)
* [Design](docs/DESIGN.md), [hardware](docs/HARDWARE.md), [base board](docs/BASE_BOARD.md) and [node firmware](docs/NODE_FIRMWARE.md)
* [Bench tests](docs/BENCH_TESTS.md), [set-up code](docs/SETUP_CODE.md) and [Bluetooth provisioning](docs/BLE_PROVISIONING.md)
* [Capability matrix: what is proven and what is not](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/CAPABILITY_MATRIX.md)
* [Project catalogue: versions and how the repositories depend on each other](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/PROJECT_CATALOG.md)
* [Changelog of this repository](CHANGELOG.md)

## 🤝 Contributing & Community

* Before a pull request run `ctest --test-dir build/host` and `python -I tools/armor_ci_validate.py`: it must pass. See [CONTRIBUTING.md](CONTRIBUTING.md) and the [code of conduct](CODE_OF_CONDUCT.md).
* The README exists in eight languages and is generated from the sources in ARMOR-DOCS: a change to the text is made there.
* A change to a message between components starts in ARMOR-COMMON.
* Do not describe as working on a board what has only been built.
* Report a vulnerability privately: [SECURITY.md](SECURITY.md). Questions, ideas and reports: electrohobby3d@gmail.com ([SUPPORT.md](SUPPORT.md)).

## 👤 AUTHOR

**JuanenRac (Electro Hobby 3D)** · electrohobby3d@gmail.com

## 📜 LICENSE

GPL-3.0-or-later - see [LICENSE](LICENSE).
