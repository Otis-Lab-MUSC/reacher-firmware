# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

---

## [Unreleased]

---

## [2.0.0] - 2025-04-08

_Changelog tracking started at this version. Earlier history not recorded._

### Added
- Unified firmware for five behavioral paradigms: Fixed Ratio (FR), Progressive Ratio (PR), Variable Interval (VI), Omission, and Pavlovian classical conditioning
- Shared `REACHERDevices` C++ library (v2.0.0): `SwitchLever`, `Cue`, `Pump`, `Laser`, `LickCircuit`, `Microscope`, `Scheduler`, `PavlovianScheduler`
- `Scheduler` engine with trigger types: `PRESS_COUNT`, `ABSENCE_TIMER`, `AVAILABILITY_WINDOW`, `MANUAL`
- `PavlovianScheduler` 5-phase trial state machine: `IDLE → ITI → CUE_ON → TRACE → REWARD`; Fisher-Yates shuffle with ≤3 consecutive same-type constraint; ITI sampled from clamped exponential distribution
- Runtime pin and pump overrides via `*_SET_PIN` command family and `SET_ACTIVE_PUMP` (221); pin range clamped to [2, 53]
- Session pause/resume with deadline shifting so paused time does not consume timeouts or absence timers
- Microscope pause/resume across session pause and split events
- 8-second watchdog timer (`wdt_enable(WDTO_8S)`) in all sketches; `wdt_reset()` at top of every `loop()`
- JSON serial protocol at 115200 baud: `*IDN?` handshake, level codes `000/001/006/007/008`, newline-delimited
- Compiled `.hex` artifacts for both Arduino UNO (`hex/uno/`) and Mega 2560 (`hex/mega/`)
- `compile.sh` script compiling all five paradigms for both board targets
- Doxygen configuration for class and function documentation
- `DeviceSet` helpers in `ReacherHelpers.{h,cpp}`: `handleCommonDeviceCommand`, `reportDeviceConfig`, `armToggleDevices`, `captureArmState`/`restoreArmState`

### Changed
- LH lever remapped from pin 12 to pin 13
- Laser gated on `sessionActive` to prevent firing outside session boundaries

### Fixed
- Signed-diff pattern (`(int32_t)(now - deadline) >= 0`) applied to all `millis()` comparisons to survive 49.7-day wrap (Bug 2.2)
- Microscope now uses `Pause`/`Resume` on session end rather than `Trigger` to prevent spurious frame capture
- `armToggleDevices` deferred 200 ms after session end to allow in-flight events to clear
- Deprecated `LASER_SET_TRACE` command (673) removed
- Laser oscillation caused by incorrect pin initialization resolved; `INDEPENDENT` mode gated on session state
- Runtime pump selection via `SET_ACTIVE_PUMP` triggers `ReconfigureChain()` to update action offsets
- Hex artifacts split by board type (`hex/uno/`, `hex/mega/`) for correct backend resolution
