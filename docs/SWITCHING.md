# Switching: the rules of the controller

`core/interlock.hpp` holds the software rules for switching two sources onto one line, kept apart from any hardware so they can be tested to exhaustion. **No published image drives any contactor; only the switching image does (below), and it has never run on a board.**

## What a transfer controller does

Two sources, `A` and `B` (for example the grid and an inverter's output), each behind a contactor whose auxiliary contact tells the node whether it is closed.

- `arm(now)`: the operator's first step. The next request to close is accepted for ten seconds (`arm_window_ms`), and then only once.
- `request(source, now)`: asks for `A`, `B` or none. **None (open) is always accepted and immediate.** A request to close is refused when the switching is not allowed (`kDisabled`), a fault is latched (`kFault`) or the node was not armed (`kNotArmed`).
- `tick(feedback, now)`: called often with the auxiliary contacts as they are.
- `coils()`: what to drive. Never both at once; a coil is energised only after both contactors were confirmed open for the whole `dead_time_ms` (two seconds by default). Going from one source to the other opens the first, waits for its contact, waits the dead time, then closes the second (break before make).
- **Faults** (all open, latched): a contactor that does not show closed within `confirm_timeout_ms` of being told (`kDidNotClose`), one that is closed when nobody asked or does not open when told (`kDidNotOpen`), both contacts closed (`kBothClosed`). A fault never clears itself: `acknowledge` works only with both contactors confirmed open.

## Commands to a switch (`core/switch_set.hpp`)

`SwitchSet` is the layer between a message and the controller: it reads a command strictly (the shared vectors of ARMOR-COMMON decide, in `tests/test_switch_set.cpp`), refuses what it should and answers. **It is linked only into the switching image** (`CONFIG_ARMOR_SWITCHING`, off by default); in every other image no node can act on a command and the state message says `switching_enabled: false`.

- **Closing is two steps.** `arm` (refused when switching is off or a fault is latched) makes the node give a one-time **token** (64 random bits) and arms the controller; `close_a` or `close_b` must carry that token within the arm window. The token is spent by the first close that presents it; a wrong token is refused (`bad_token`) and withdraws the arm; a close with no arm is `not_armed`. An old command that turns up again finds no token to match and moves nothing.
- **`open` needs no token and no arm**, is accepted with switching off or a fault latched, and withdraws any arm. `acknowledge` clears a latched fault only with both contactors confirmed open (`not_confirmed_open` otherwise).
- **Refusals** are the shared list: `disabled`, `fault`, `not_armed`, `not_confirmed_open`, `unknown_switch`, `bad_token`, `not_supported` (no source of randomness). A command that is not the contract's (or is for another node) gets no answer at all.
- **The state message** carries `switches` (`SwitchSet::switches_json`): the contacts, what they confirm, what is wanted, closing, armed, the fault. `selected` comes from the contacts, never from what was asked; `armed`, `closing` and `wanted` are silenced while a fault is latched.
- **The layers above it:** ARMOR-SERVER only publishes a command when it was turned on (`ARMOR_ELECTRICAL_SWITCHING=1`), the node reported itself as allowed to switch and has that switch, and the caller is an administrator; the broker's ACL has to allow the topic (`scripts/mqtt_identity.sh electrical-switching`). Three independent switches, all off by default.

## The switching image

`tools/build_node.sh` builds it with `ARMOR_SWITCHING=1` (`ARMOR_SWITCHING=1 bash tools/build_node.sh generic s3-eth`): the generic image plus `main/switching.cpp`, named `generic-<board>-switching.bin`. It is for a bench and is never published; `firmware-build.yml` never sets it, and the manual workflow `switching-build.yml` only compiles it and keeps the file for a week.

- **One switch, called `transfer`**, with two coil pins and two auxiliary-contact pins. The menu "Switching image" of the firmware gives the defaults: coils on GPIO 4 and 5, contacts on GPIO 6 and 7, the polarity of each, the dead time (2000 ms), the time a contactor may take to show what it was told (1000 ms) and the arm window (10000 ms). The four pins must not be a pin set for a relay, an analog input, the serial line or the radio in the panel, nor GPIO 0.
- **Commands** arrive on `armor/electrical/<node>/command` and the answer goes out on `armor/electrical/<node>/result`, exactly as above; with a `command_key` in the settings only a command the server signed is taken. The state message carries `switching_enabled: true` and the `switches` array, in the same message as the readings.
- **Fail to open.** Both coil levels are set to open before the pins become outputs, are written again every 20 ms, and are never both energised whatever the controller says. A node that is paused or put in standby opens every switch and refuses to close until it works again. A task that hangs is restarted by the watchdog and the pins fall to open with the restart; the board must pull both coil drivers to their open level with a resistor of its own.
- **What was run:** the image compiles for both boards in the ESP-IDF container, and `SwitchSet::open_all` and the switch report of the message are tested on a computer. **Nothing has run on a board:** the proof is to wire two contactors with auxiliary contacts to a bench node and run the scripted scenarios of `tests/test_switch_set.cpp` against it (close, change over, a welded contactor, a command replayed, a node paused while closed).

## What the tests check

Scripted scenarios (switching off, arming, a transfer both ways, a welded contactor, one that will not close, both closed, one that falls out, something closed by itself) and 135,000 random steps with contactors of random speed that weld, stick open and move by themselves: never two coils, nothing energised with the switching off, every energised coil preceded by the whole dead time with both contacts open, a fault always means all coils open, and a request to open drops every coil at the next tick. The commands are tested the same way (`tests/test_switch_set.cpp`, 40 rounds of random commands, noise and contactor faults: never two coils, nothing energised with switching off, a fault always opens; and the reader agrees with every shared vector), and a broken token check is caught.

## What it is not

Not a protection relay, not a certified safety function, and not a substitute for a mechanical interlock. The contactors are chosen and wired by an installer; this only decides when to ask them to move.
