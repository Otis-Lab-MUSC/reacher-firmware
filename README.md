# REACHER — Arduino Firmware

**Operant conditioning controller firmware for Arduino UNO (ATmega328P)**

[![Version](https://img.shields.io/badge/version-2.0.0-blue)](https://github.com/Otis-Lab-MUSC/REACHER-Firmware)
[![Platform](https://img.shields.io/badge/platform-Arduino%20UNO-teal)](https://www.arduino.cc/)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)
[![REACHER Suite](https://img.shields.io/badge/REACHER_Suite-member-orange)](https://github.com/Otis-Lab-MUSC)

*Written by*: Joshua Boquiren

[![](https://img.shields.io/badge/@thejoshbq-grey?style=flat&logo=github)](https://github.com/thejoshbq)

---

## Overview

This repository contains Arduino C++ firmware implementing five behavioral paradigms for head-fixed rodent operant conditioning experiments. Each paradigm is compiled into a standalone `.hex` file that can be uploaded to an Arduino UNO via the REACHER frontend UI or manually via `arduino-cli`/`avrdude`.

**Paradigms:**
- **Fixed Ratio (FR)** — reward after N active lever presses
- **Progressive Ratio (PR)** — reward threshold increases by a configurable step after each delivery
- **Variable Interval (VI)** — reward available during a random window within each fixed interval
- **Omission** — reward delivered after the animal withholds pressing for a specified duration
- **Pavlovian** — classical conditioning with CS+/CS- trials, cues, and probabilistic reward delivery

---

## Role in the REACHER Ecosystem

The firmware is the embedded hardware layer. It directly controls all peripheral devices (levers, syringe pumps, tone cues, optogenetic lasers, lick detection circuits, and microscope synchronization triggers) and communicates with the Python backend over USB serial using JSON messages at 115200 baud.

```
Arduino Firmware ◄──USB Serial (115200 baud)──► Python Backend ◄──► React Frontend
   (firmware/)          JSON messages                (server/)       (web/)
```

The firmware receives configuration commands from the backend, executes behavioral paradigms autonomously, and streams all events (lever presses, pump activations, lick detections, frame timestamps) back in real time.

---

## Hardware Requirements

| Component | Description |
|---|---|
| Arduino UNO | ATmega328P microcontroller |
| USB cable | Type-A to Type-B for serial connection |
| Levers | Two momentary switches (right-hand and left-hand) |
| Syringe pumps | Two relay-driven pumps (primary and secondary) |
| Tone speakers | Two speakers for auditory cues (PWM-driven) |
| Laser | Optogenetic laser module (PWM, up to 40 Hz square-wave) |
| Lick circuit | Capacitive or resistive lick detection circuit |
| Microscope | Two-photon microscope with TTL trigger input and frame timestamp output |

---

## Pin Configuration

All pin assignments are defined in `libraries/REACHERDevices/src/Pins.h`:

| Pin | Constant | Direction | Description |
|---|---|---|---|
| 2 | `PIN_MICROSCOPE_TS` | INPUT (INT0) | Microscope frame timestamp — rising-edge ISR captures frame times |
| 3 | `PIN_CUE` | OUTPUT (PWM) | Primary tone cue speaker |
| 4 | `PIN_PUMP` | OUTPUT | Primary syringe pump relay |
| 5 | `PIN_LICK_CIRCUIT` | INPUT_PULLUP | Lick detection circuit |
| 6 | `PIN_LASER` | OUTPUT (PWM) | Optogenetic laser |
| 7 | `PIN_CUE_2` | OUTPUT (PWM) | Secondary tone cue speaker |
| 8 | `PIN_PUMP_2` | OUTPUT | Secondary syringe pump relay |
| 9 | `PIN_MICROSCOPE_TRIG` | OUTPUT | Microscope trigger pulse (50 ms HIGH) |
| 10 | `PIN_LEVER_RH` | INPUT_PULLUP | Right-hand lever |
| 12 | `PIN_LEVER_LH` | INPUT_PULLUP | Left-hand lever |

---

## Paradigms

### Fixed Ratio (FR)

**Description:** The animal must complete N active lever presses to earn a reward. Each reward delivery consists of a cue tone, a pump infusion, and an optional laser pulse, followed by a timeout period during which presses are logged but not reinforced.

**Press classifications:**
- `ACTIVE` — press on the reinforced lever outside the timeout period; counted toward the ratio
- `TIMEOUT` — press during the post-reward timeout period
- `INACTIVE` — press on the non-reinforced lever

**Default configuration** (`fr/Config.h`):

| Parameter | Default | Description |
|---|---|---|
| Ratio | 1 | Number of active presses required per reward |
| Cue frequency | 8000 Hz | Primary tone frequency |
| Cue duration | 1600 ms | Tone duration |
| Pump duration | 2000 ms | Infusion duration |
| Laser frequency | 40 Hz | Laser oscillation frequency |
| Laser duration | 5000 ms | Laser pulse duration |
| Timeout | 20000 ms | Post-reward timeout interval |
| Trace interval | 0 ms | Delay between cue offset and pump onset |

**Reward chain:** Cue (immediate) → Pump (after cue + trace) → Laser (after cue + trace) → Timeout (immediate)

---

### Progressive Ratio (PR)

**Description:** Like Fixed Ratio, but the press threshold increases by a configurable step after each reward delivery using arithmetic progression (e.g., 1, 2, 3, 4... with step=1).

**Press classifications:** Same as FR (ACTIVE, TIMEOUT, INACTIVE).

**Additional configuration:**

| Parameter | Default | Description |
|---|---|---|
| PR step | 1 | Arithmetic increment added to the threshold after each reward |

All other defaults are the same as FR. The threshold increases as: `threshold += step` after each reward.

---

### Variable Interval (VI)

**Description:** A fixed-length interval contains a randomly placed availability window. A lever press during the availability window triggers the reward. Presses outside the window are logged but not reinforced.

**Press classifications:** Same as FR (ACTIVE, TIMEOUT, INACTIVE).

**Additional configuration:**

| Parameter | Default | Description |
|---|---|---|
| VI interval | 15000 ms | Total interval length (window is placed randomly within it) |

The availability window start and end positions are sampled uniformly within each interval. After a reward or interval expiry, a new interval begins with a new random window.

---

### Omission

**Description:** The animal is rewarded for *withholding* lever presses. If no active press occurs for the specified duration, the reward fires. Any active press resets the absence timer. There is no timeout period.

**Press classifications:**
- `ACTIVE` — press on the reinforced lever; resets the absence timer
- `INACTIVE` — press on the non-reinforced lever

**Configuration:**

| Parameter | Default | Description |
|---|---|---|
| Omission interval | 20000 ms | Duration of press absence required to trigger reward |
| Cue frequency | 8000 Hz | Primary tone frequency |
| Cue duration | 1600 ms | Tone duration |
| Pump duration | 2000 ms | Infusion duration |
| Laser frequency | 40 Hz | Laser oscillation frequency |
| Laser duration | 5000 ms | Laser pulse duration |
| Timeout | 0 ms | No timeout in omission paradigm |

**Reward chain:** Cue (immediate) → Pump (immediate) → Laser (immediate)

---

### Pavlovian (Classical Conditioning)

**Description:** A trial-based paradigm with CS+ and CS- trials. CS+ trials present a continuous tone followed by reward delivery. CS- trials present a pulsed tone with no reward (by default). Trial order is randomized using Fisher-Yates shuffle with a constraint of no more than 3 consecutive same-type trials. Inter-trial intervals (ITI) are sampled from a clamped exponential distribution.

This paradigm uses a dedicated `PavlovianScheduler` state machine (not the shared `Scheduler` engine) with five phases: `IDLE` → `ITI` → `CUE_ON` → `TRACE` → `REWARD`.

**Lever behavior:** Both levers are set as reinforced. Presses are logged as ACTIVE but do not affect trial progression or reward delivery.

**No laser device** is used in the Pavlovian paradigm.

**Default configuration** (`pavlovian/pavlovian.ino`):

| Parameter | Default | Command Code | Description |
|---|---|---|---|
| CS+ count | 50 | 208 | Number of CS+ trials |
| CS- count | 50 | 209 | Number of CS- trials |
| CS+ frequency | 12000 Hz | 210 | Tone frequency for CS+ |
| CS- frequency | 3000 Hz | 211 | Tone frequency for CS- |
| CS+ reward probability | 100% | 206 | Probability of reward on CS+ trials |
| CS- reward probability | 0% | 207 | Probability of reward on CS- trials |
| Counterbalance | false | 212 | Swap CS+ and CS- cue assignments |
| Cue duration | 2000 ms | 213 | Duration of conditioned stimulus |
| Trace interval | 1000 ms | 214 | Delay between CS offset and reward |
| Consumption period | 3000 ms | 215 | Time allowed for reward consumption |
| ITI mean | 30000 ms | 216 | Mean inter-trial interval |
| ITI min | 10000 ms | 217 | Minimum ITI |
| ITI max | 90000 ms | 218 | Maximum ITI |
| Pulse config | 200/200 ms | 219 | CS- pulsed tone ON/OFF durations |

---

## Shared C++ Library (REACHERDevices)

All paradigms share a common device library at `libraries/REACHERDevices/` (v2.0.0, `avr` architecture).

### Class Hierarchy

```
Device (base)
├── SwitchLever    — Lever monitoring with debounce, press classification, timeout tracking
├── Cue            — Tone generation via Arduino tone(), continuous or pulsed modes
├── Pump           — Relay-driven syringe pump activation
├── Laser          — PWM laser with contingent/independent modes, square-wave oscillation
└── LickCircuit    — Lick detection with debounce and event logging

Microscope (standalone) — Frame timestamp capture via INT0 ISR + trigger pulse output

Scheduler          — Contingency engine for operant paradigms (FR, PR, VI, Omission)
├── Trigger        — Event sources: PRESS_COUNT, ABSENCE_TIMER, AVAILABILITY_WINDOW, MANUAL
├── Chain          — Ordered sequence of Actions (up to 6 steps)
├── Action         — ACTIVATE_DEVICE, SET_TIMEOUT, RESET_TRIGGER, NONE
└── PendingAction  — Time-delayed action execution queue (up to 8 pending)

PavlovianScheduler — Trial-based state machine for classical conditioning (standalone)

DeviceSet          — Struct grouping all device pointers for helper functions
```

### Key Classes

**Device** (base class) — `Device.h`
- Constructor: `Device(int8_t pin, uint8_t mode, const char* device)`
- Common interface: `ArmToggle(bool)`, `SetOffset(uint32_t)`, `Pin()`, `Armed()`

**SwitchLever** — 20 ms debounce, press/release callbacks, active/inactive/timeout classification

**Cue** — Uses Arduino `tone()` (Timer2). Supports continuous and pulsed modes (for CS- in Pavlovian)

**Pump** — Drives relay HIGH for the configured infusion duration

**Laser** — Two modes: `CONTINGENT` (fires only via chain trigger) or `INDEPENDENT` (free-running square-wave cycle). Frequency of 1 Hz = continuous ON; >1 Hz = oscillation

**LickCircuit** — 20 ms debounce, logs lick events with start/end timestamps

**Microscope** — Manages two pins: trigger output (50 ms HIGH pulse) and INT0 ISR for frame timestamp capture

### Scheduler (Operant Paradigms)

The `Scheduler` class implements a trigger-chain-action system for the four operant paradigms:

**Trigger types:**
| Type | Paradigm | Description |
|---|---|---|
| `PRESS_COUNT` | FR, PR | Fire chain after N active presses |
| `ABSENCE_TIMER` | Omission | Fire chain after N ms with no active press |
| `AVAILABILITY_WINDOW` | VI | Fire chain if pressed during random window |
| `MANUAL` | All | Fire chain via serial test command |

**Action types:**
| Type | Description |
|---|---|
| `ACTIVATE_DEVICE` | Call `device->Activate()` with offset delay |
| `SET_TIMEOUT` | Set lever timeout end time |
| `RESET_TRIGGER` | Reset trigger press count to zero |
| `NONE` | No-op placeholder |

### Press Classification

| Class | Description |
|---|---|
| `ACTIVE` | Press on reinforced lever outside timeout — counted toward trigger |
| `INACTIVE` | Press on non-reinforced lever |
| `TIMEOUT` | Press during post-reward timeout period |

---

## Serial Protocol

| Parameter | Value |
|---|---|
| Baud rate | 115200 bps |
| Timeout | 10 ms |
| Format | Newline-delimited JSON (CR-LF) |
| Identification | `*IDN?` query → JSON response with sketch, version, baud rate |

### Event levels (firmware → backend)

| Level | Meaning |
|---|---|
| `000` | Configuration dump (firmware identification, device settings) |
| `001` | State changes (arm/disarm notifications) |
| `006` | Error messages |
| `007` | Behavioral events (presses, infusions, licks, device activations) |
| `008` | Microscope frame timestamps |

### Identification response format

```json
{
  "level": "000",
  "device": "CONTROLLER",
  "sketch": "fr.ino",
  "version": "v2.0.0",
  "baud_rate": 115200,
  "schedule": "FIXED_RATIO"
}
```

### Command codes (backend → firmware)

| Range | Target | Key Codes |
|---|---|---|
| 100–105 | Controller | SESSION_END (100), SESSION_START (101), IDENTIFY (102), TEST_CHAIN (103), TEST_MODE (104), SESSION_PAUSE (105) |
| 201–220 | Session setup | SET_RATIO (201), SET_PARADIGM (202), SET_OMISSION_INTERVAL (203), SET_VI_INTERVAL (204), SET_PR_STEP (205), Pavlovian params (206–219), SET_TRACE_INTERVAL (220) |
| 300–382 | Cue | Primary: ARM (301), DISARM (300), TEST (303), FREQ (371), DUR (372). Secondary: ARM (311), DISARM (310), TEST (313), FREQ (381), DUR (382) |
| 400–482 | Pump | Primary: ARM (401), DISARM (400), TEST (403), DUR (472). Secondary: ARM (411), DISARM (410), TEST (413), DUR (482) |
| 500–501 | Lick circuit | DISARM (500), ARM (501) |
| 600–682 | Laser | ARM (601), DISARM (600), TEST (603), FREQ (671), DUR (672), CONTINGENT (681), INDEPENDENT (682) |
| 900–903 | Microscope | DISARM (900), ARM (901), TEST (903) |
| 1000–1081 | Right lever | ARM (1001), DISARM (1000), TIMEOUT (1074), RATIO (1075), INACTIVE (1080), ACTIVE (1081) |
| 1300–1381 | Left lever | ARM (1301), DISARM (1300), TIMEOUT (1374), RATIO (1375), INACTIVE (1380), ACTIVE (1381) |

---

## Compiling

### Prerequisites

- `arduino-cli` installed and on your PATH
- Arduino AVR board package: `arduino-cli core install arduino:avr`

### Compilation

```bash
cd reacher-firmware
./compile.sh
```

The script compiles all five paradigms (`fr`, `pr`, `vi`, `omission`, `pavlovian`) using the local `libraries/` directory and outputs `.hex` files to the `hex/` directory:

```
hex/
├── fr.hex
├── pr.hex
├── vi.hex
├── omission.hex
└── pavlovian.hex
```

Target board: `arduino:avr:uno` (Arduino UNO, ATmega328P).

---

## Uploading to Arduino

### Via the REACHER frontend

The easiest method. In the REACHER UI:
1. Select a COM port and create a session
2. Choose a paradigm from the dropdown
3. Click "Upload Firmware"

The backend handles the upload via `avrdude` automatically.

### Manually via arduino-cli

```bash
arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:uno --input-file hex/fr.hex
```

### Manually via avrdude

```bash
avrdude -p atmega328p -c arduino -P /dev/ttyUSB0 -b 115200 -U flash:w:hex/fr.hex:i
```

---

## Data Integrity Measures

| Mechanism | Details |
|---|---|
| Debouncing | SwitchLever: 20 ms, LickCircuit: 20 ms |
| Interrupt-driven frame capture | Microscope frame timestamps captured via rising-edge INT0 ISR on pin 2 |
| Real-time serial logging | All events are sent immediately over serial with session-relative timestamps |
| JSON format | Structured data prevents parsing ambiguity |

---

## Project Structure

```
reacher-firmware/
├── compile.sh                    # Compilation script for all 5 paradigms
├── Doxyfile                      # Doxygen configuration
├── fr/                           # Fixed Ratio paradigm
│   ├── fr.ino
│   └── Config.h
├── pr/                           # Progressive Ratio paradigm
│   ├── pr.ino
│   └── Config.h
├── vi/                           # Variable Interval paradigm
│   ├── vi.ino
│   └── Config.h
├── omission/                     # Omission paradigm
│   ├── omission.ino
│   └── Config.h
├── pavlovian/                    # Classical conditioning paradigm
│   ├── pavlovian.ino
│   ├── PavlovianScheduler.h
│   └── PavlovianScheduler.cpp
├── libraries/
│   └── REACHERDevices/           # Shared device library (v2.0.0)
│       ├── library.properties
│       └── src/
│           ├── Pins.h            # Pin assignments
│           ├── Commands.h        # Serial command codes
│           ├── Device.h/.cpp     # Base class
│           ├── SwitchLever.h/.cpp
│           ├── Cue.h/.cpp
│           ├── Pump.h/.cpp
│           ├── Laser.h/.cpp
│           ├── LickCircuit.h/.cpp
│           ├── Microscope.h/.cpp
│           ├── Scheduler.h/.cpp  # Trigger/chain/action engine
│           ├── Trigger.h
│           ├── Action.h
│           └── ReacherHelpers.h/.cpp
├── hex/                          # Compiled hex binaries
└── docs/                         # Doxygen-generated documentation
```

---

## API Documentation

Full Doxygen-generated class and function documentation is available in the `docs/` directory. To regenerate:

```bash
doxygen Doxyfile
```

---

## Building the Full REACHER Executable

See the [top-level README](../README.md) for instructions on building the complete standalone REACHER application, which bundles the firmware hex files, frontend, and backend into a single executable.

---

## License

This project is licensed under the MIT License.

## Contact

Joshua Boquiren — [thejoshbq@proton.me](mailto:thejoshbq@proton.me)

[GitHub: Otis-Lab-MUSC/REACHER-Firmware](https://github.com/Otis-Lab-MUSC/REACHER-Firmware)
