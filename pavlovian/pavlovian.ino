/**
 * @file pavlovian.ino
 * @brief Main sketch for REACHER v2.0 Pavlovian (classical conditioning) controller.
 *
 * @mainpage REACHER v2.0 Pavlovian Firmware
 *
 * Classical conditioning controller firmware for Arduino Uno (ATmega328P, 2KB RAM, 32KB Flash).
 *
 * **Baud rate:** 115200
 *
 * **Paradigm:** Pavlovian (timer-driven CS+/CS- trials with ITI, cue, trace, and reward phases).
 *
 * **Architecture:** The PavlovianScheduler drives a trial-based state machine
 * (ITI -> CUE -> TRACE -> REWARD). Lever presses are logged but do not affect
 * trial progression. Two cues (CS+/CS-) and two pumps are used. No laser or
 * operant triggers.
 *
 * **JSON protocol levels:**
 * - 000 — Settings / configuration dump
 * - 001 — Arm / disarm state changes
 * - 006 — Error messages
 * - 007 — Behavioral events (presses, licks, device activations, trial events)
 * - 008 — Microscope frame timestamps
 */

#include <Arduino.h>
#include <ArduinoJson.h>

#include <Commands.h>
#include <Pins.h>
#include <Device.h>
#include <SwitchLever.h>
#include <LickCircuit.h>
#include <Cue.h>
#include <Pump.h>
#include <Microscope.h>
#include <ReacherHelpers.h>
#include "PavlovianScheduler.h"

// Default parameter values
static constexpr uint32_t DEFAULT_CUE_FREQUENCY = 8000;
static constexpr uint32_t DEFAULT_CUE_DURATION  = 2000;
static constexpr uint32_t DEFAULT_PUMP_DURATION  = 2000;

// Pavlovian paradigm parameters
uint8_t  PAV_CS_PLUS_COUNT   = 50;
uint8_t  PAV_CS_MINUS_COUNT  = 50;
uint32_t PAV_CS_PLUS_FREQ    = 12000;
uint32_t PAV_CS_MINUS_FREQ   = 3000;
uint8_t  PAV_CS_PLUS_PROB    = 100;
uint8_t  PAV_CS_MINUS_PROB   = 0;
bool     PAV_COUNTERBALANCE  = false;
uint16_t PAV_CUE_DURATION    = 2000;
uint16_t PAV_TRACE_INTERVAL  = 1000;
uint16_t PAV_CONSUMPTION_MS  = 3000;
uint32_t PAV_ITI_MEAN        = 30000;
uint32_t PAV_ITI_MIN         = 10000;
uint32_t PAV_ITI_MAX         = 90000;

// Device instances
SwitchLever rLever(PIN_LEVER_RH, "RH", DeviceType::LEVER_RH);
SwitchLever lLever(PIN_LEVER_LH, "LH", DeviceType::LEVER_LH);

Cue         cue(PIN_CUE, DEFAULT_CUE_FREQUENCY, DEFAULT_CUE_DURATION);
Cue         cue2(PIN_CUE_2, DEFAULT_CUE_FREQUENCY, DEFAULT_CUE_DURATION);
Pump        pump(PIN_PUMP, DEFAULT_PUMP_DURATION);
Pump        pump2(PIN_PUMP_2, DEFAULT_PUMP_DURATION);
LickCircuit lickCircuit(PIN_LICK_CIRCUIT);
Microscope  microscope(PIN_MICROSCOPE_TRIG, PIN_MICROSCOPE_TS);

/// Central scheduler instance.
PavlovianScheduler scheduler;

DeviceSet devices = { &rLever, &lLever, &cue, &cue2, &pump, &pump2, &lickCircuit, nullptr, &microscope };

// Session timestamps
uint32_t SESSION_START_TIMESTAMP;
uint32_t SESSION_END_TIMESTAMP;

// Forward declarations
void ParseCommands();
void StartSession();
void EndSession();
void ReconfigureScheduler();
void SendIdentification();

/// @brief Callback: forward lever press to scheduler.
void onLeverPress(DeviceType source, uint32_t timestamp) {
  scheduler.OnInputEvent(source, timestamp);
}

/// @brief Callback: forward lever release to scheduler.
void onLeverRelease(DeviceType source) {
  scheduler.OnInputRelease(source);
}

/// @brief Send SCPI-style identification JSON (reusable for boot, *IDN?, and Cmd::IDENTIFY).
void SendIdentification() {
  Serial.println(F("{\"level\":\"000\",\"device\":\"CONTROLLER\",\"sketch\":\"pavlovian.ino\",\"version\":\"v2.0.0\",\"baud_rate\":115200,\"schedule\":\"PAVLOVIAN\"}"));
}

/// @brief Arduino setup — initialize serial, register devices, configure Pavlovian defaults.
void setup() {
  delay(100);
  Serial.begin(115200);
  Serial.setTimeout(10);
  delay(100);

  cue.Jingle();

  // Register devices with scheduler
  scheduler.RegisterLever(&rLever, DeviceType::LEVER_RH);
  scheduler.RegisterLever(&lLever, DeviceType::LEVER_LH);
  scheduler.RegisterLickCircuit(&lickCircuit);
  scheduler.RegisterCue(&cue);
  scheduler.RegisterCue2(&cue2);
  scheduler.RegisterPump(&pump);
  scheduler.RegisterPump2(&pump2);
  scheduler.RegisterMicroscope(&microscope);

  // Set input callbacks
  rLever.SetCallback(onLeverPress);
  rLever.SetReleaseCallback(onLeverRelease);
  lLever.SetCallback(onLeverPress);
  lLever.SetReleaseCallback(onLeverRelease);

  // Both levers are reinforced (presses logged as ACTIVE)
  rLever.SetActiveLever(true);
  lLever.SetActiveLever(true);

  // Configure default Pavlovian parameters
  ReconfigureScheduler();

  // Send setup JSON
  SendIdentification();
}

/// @brief Arduino main loop — poll inputs, tick scheduler, handle serial commands.
void loop() {
  uint32_t currentTimestamp = millis();

  // Poll inputs
  rLever.Monitor(currentTimestamp);
  lLever.Monitor(currentTimestamp);
  lickCircuit.Monitor(currentTimestamp);

  // Scheduler: tick trial state machine and output devices
  scheduler.Update(currentTimestamp);

  // Microscope frame handling
  microscope.HandleFrameSignal();

  // Process serial commands
  ParseCommands();
}

/// @brief Deserialize JSON serial input and dispatch to device/session handlers.
void ParseCommands() {
  if (Serial.available() > 0) {
    JsonDocument inputJson;
    char buf[128];
    int len = Serial.readBytesUntil('\n', buf, sizeof(buf) - 1);
    buf[len] = '\0';

    // SCPI-style identification query (not valid JSON, must check before deserialization)
    if (len >= 5 && memcmp(buf, "*IDN?", 5) == 0) {
      SendIdentification();
      return;
    }

    DeserializationError error = deserializeJson(inputJson, buf);

    if (error) {
      Serial.print(F("{\"level\":\"006\",\"desc\":\""));
      Serial.print(error.f_str());
      Serial.println(F("\"}"));
      while (Serial.available() > 0) Serial.read();
      return;
    }

    if (!inputJson["cmd"].isNull()) {
      int command = inputJson["cmd"];
      switch (command) {

        // RH lever commands
        case Cmd::LEVER_RH_ARM:    rLever.ArmToggle(true); break;
        case Cmd::LEVER_RH_DISARM: rLever.ArmToggle(false); break;

        // LH lever commands
        case Cmd::LEVER_LH_ARM:    lLever.ArmToggle(true); break;
        case Cmd::LEVER_LH_DISARM: lLever.ArmToggle(false); break;

        // Cue commands
        case Cmd::CUE_ARM:           cue.ArmToggle(true); break;
        case Cmd::CUE_DISARM:        cue.ArmToggle(false); break;
        case Cmd::CUE_SET_FREQUENCY:
          PAV_CS_PLUS_FREQ = inputJson["frequency"]; ReconfigureScheduler();
          logParamChange(F("CUE"), F("frequency"), PAV_CS_PLUS_FREQ); break;
        case Cmd::CUE_TEST:
          cue.Test(millis());
          logParamChange(F("CUE"), F("test"), F("FIRED")); break;
        case Cmd::CUE_SET_DURATION:
          PAV_CUE_DURATION = inputJson["duration"]; ReconfigureScheduler();
          logParamChange(F("CUE"), F("duration"), (uint32_t)PAV_CUE_DURATION); break;

        // Secondary cue commands
        case Cmd::CUE2_ARM:           cue2.ArmToggle(true); break;
        case Cmd::CUE2_DISARM:        cue2.ArmToggle(false); break;
        case Cmd::CUE2_SET_FREQUENCY:
          PAV_CS_MINUS_FREQ = inputJson["frequency"]; ReconfigureScheduler();
          logParamChange(F("CUE2"), F("frequency"), PAV_CS_MINUS_FREQ); break;
        case Cmd::CUE2_TEST:
          cue2.Test(millis());
          logParamChange(F("CUE2"), F("test"), F("FIRED")); break;
        case Cmd::CUE2_SET_DURATION:
          cue2.SetDuration(inputJson["duration"]); ReconfigureScheduler();
          logParamChange(F("CUE2"), F("duration"), (uint32_t)inputJson["duration"]); break;

        // Pump commands
        case Cmd::PUMP_ARM:          pump.ArmToggle(true); break;
        case Cmd::PUMP_DISARM:       pump.ArmToggle(false); break;
        case Cmd::PUMP_TEST:
          pump.Test(millis());
          logParamChange(F("PUMP"), F("test"), F("FIRED")); break;
        case Cmd::PUMP_SET_DURATION:
          pump.SetDuration(inputJson["duration"]);
          logParamChange(F("PUMP"), F("duration"), (uint32_t)inputJson["duration"]); break;

        // Secondary pump commands
        case Cmd::PUMP2_ARM:          pump2.ArmToggle(true); break;
        case Cmd::PUMP2_DISARM:       pump2.ArmToggle(false); break;
        case Cmd::PUMP2_TEST:
          pump2.Test(millis());
          logParamChange(F("PUMP2"), F("test"), F("FIRED")); break;
        case Cmd::PUMP2_SET_DURATION:
          pump2.SetDuration(inputJson["duration"]);
          logParamChange(F("PUMP2"), F("duration"), (uint32_t)inputJson["duration"]); break;

        // Lick circuit commands
        case Cmd::LICK_ARM:    lickCircuit.ArmToggle(true); break;
        case Cmd::LICK_DISARM: lickCircuit.ArmToggle(false); break;

        // Microscope commands
        case Cmd::MICROSCOPE_ARM:    microscope.ArmToggle(true); break;
        case Cmd::MICROSCOPE_DISARM: microscope.ArmToggle(false); break;
        case Cmd::MICROSCOPE_TEST:
          microscope.Trigger();
          logParamChange(F("MICROSCOPE"), F("test"), F("FIRED")); break;

        // Pavlovian paradigm commands
        case Cmd::PAV_CS_PLUS_PROB:
          PAV_CS_PLUS_PROB = inputJson["probability"]; ReconfigureScheduler();
          logParamChange(F("CONTROLLER"), F("cs_plus_prob"), (uint32_t)PAV_CS_PLUS_PROB); break;
        case Cmd::PAV_CS_MINUS_PROB:
          PAV_CS_MINUS_PROB = inputJson["probability"]; ReconfigureScheduler();
          logParamChange(F("CONTROLLER"), F("cs_minus_prob"), (uint32_t)PAV_CS_MINUS_PROB); break;
        case Cmd::PAV_CS_PLUS_COUNT:
          PAV_CS_PLUS_COUNT = inputJson["count"]; ReconfigureScheduler();
          logParamChange(F("CONTROLLER"), F("cs_plus_count"), (uint32_t)PAV_CS_PLUS_COUNT); break;
        case Cmd::PAV_CS_MINUS_COUNT:
          PAV_CS_MINUS_COUNT = inputJson["count"]; ReconfigureScheduler();
          logParamChange(F("CONTROLLER"), F("cs_minus_count"), (uint32_t)PAV_CS_MINUS_COUNT); break;
        case Cmd::PAV_CS_PLUS_FREQ:
          PAV_CS_PLUS_FREQ = inputJson["frequency"]; ReconfigureScheduler();
          logParamChange(F("CONTROLLER"), F("cs_plus_freq"), PAV_CS_PLUS_FREQ); break;
        case Cmd::PAV_CS_MINUS_FREQ:
          PAV_CS_MINUS_FREQ = inputJson["frequency"]; ReconfigureScheduler();
          logParamChange(F("CONTROLLER"), F("cs_minus_freq"), PAV_CS_MINUS_FREQ); break;
        case Cmd::PAV_COUNTERBALANCE:
          PAV_COUNTERBALANCE = inputJson["counterbalance"]; ReconfigureScheduler();
          logParamChange(F("CONTROLLER"), F("counterbalance"), PAV_COUNTERBALANCE); break;
        case Cmd::PAV_CUE_DURATION:
          PAV_CUE_DURATION = inputJson["duration"]; ReconfigureScheduler();
          logParamChange(F("CONTROLLER"), F("cue_duration"), (uint32_t)PAV_CUE_DURATION); break;
        case Cmd::PAV_TRACE_INTERVAL:
          PAV_TRACE_INTERVAL = inputJson["interval"]; ReconfigureScheduler();
          logParamChange(F("CONTROLLER"), F("trace_interval"), (uint32_t)PAV_TRACE_INTERVAL); break;
        case Cmd::PAV_CONSUMPTION:
          PAV_CONSUMPTION_MS = inputJson["duration"]; ReconfigureScheduler();
          logParamChange(F("CONTROLLER"), F("consumption"), (uint32_t)PAV_CONSUMPTION_MS); break;
        case Cmd::PAV_ITI_MEAN:
          PAV_ITI_MEAN = inputJson["iti_mean"]; ReconfigureScheduler();
          logParamChange(F("CONTROLLER"), F("iti_mean"), PAV_ITI_MEAN); break;
        case Cmd::PAV_ITI_MIN:
          PAV_ITI_MIN = inputJson["iti_min"]; ReconfigureScheduler();
          logParamChange(F("CONTROLLER"), F("iti_min"), PAV_ITI_MIN); break;
        case Cmd::PAV_ITI_MAX:
          PAV_ITI_MAX = inputJson["iti_max"]; ReconfigureScheduler();
          logParamChange(F("CONTROLLER"), F("iti_max"), PAV_ITI_MAX); break;
        case Cmd::PAV_PULSE_CONFIG: {
          uint16_t onMs = inputJson["pulse_on"] | (uint16_t)200;
          uint16_t offMs = inputJson["pulse_off"] | (uint16_t)200;
          cue2.SetPulsed(true, onMs, offMs);
          logParamChange(F("CUE2"), F("pulse_on"), (uint32_t)onMs);
          logParamChange(F("CUE2"), F("pulse_off"), (uint32_t)offMs);
          break;
        }

        // Controller commands
        case Cmd::SESSION_START:
          StartSession(); setDeviceTimestampOffset(devices, SESSION_START_TIMESTAMP); break;
        case Cmd::SESSION_END:
          EndSession(); armToggleDevices(devices, false); break;
        case Cmd::IDENTIFY:
          SendIdentification(); break;
        case Cmd::SESSION_PAUSE: {
          bool paused = inputJson["paused"] | false;
          scheduler.SetPaused(paused, millis());
          logParamChange(F("CONTROLLER"), F("session_paused"), paused);
          break;
        }

        // Error
        default:
          Serial.println(F("{\"level\":\"006\",\"desc\":\"Command not found\"}"));
          break;
      }
    }
  }
}

/// @brief Push current parameter globals into the scheduler.
void ReconfigureScheduler() {
  scheduler.Configure(
    PAV_CS_PLUS_COUNT, PAV_CS_MINUS_COUNT,
    PAV_CS_PLUS_FREQ, PAV_CS_MINUS_FREQ,
    PAV_CUE_DURATION, PAV_TRACE_INTERVAL, PAV_CONSUMPTION_MS,
    PAV_ITI_MEAN, PAV_ITI_MIN, PAV_ITI_MAX,
    PAV_CS_PLUS_PROB, PAV_CS_MINUS_PROB,
    PAV_COUNTERBALANCE);
}

/// @brief Begin a session: trigger microscope, initialize scheduler, emit settings JSON.
void StartSession() {
  SESSION_START_TIMESTAMP = millis();
  microscope.Trigger();
  scheduler.StartSession(SESSION_START_TIMESTAMP);

  Serial.println(F("{\"level\":\"007\",\"device\":\"CONTROLLER\",\"event\":\"START\",\"timestamp\":0}"));

  // Send Pavlovian settings
  Serial.print(F("{\"level\":\"000\",\"device\":\"CONTROLLER\",\"paradigm\":\"PAVLOVIAN\",\"cs_plus_count\":"));
  Serial.print(PAV_CS_PLUS_COUNT);
  Serial.print(F(",\"cs_minus_count\":"));
  Serial.print(PAV_CS_MINUS_COUNT);
  Serial.print(F(",\"cs_plus_prob\":"));
  Serial.print(PAV_CS_PLUS_PROB);
  Serial.print(F(",\"cs_minus_prob\":"));
  Serial.print(PAV_CS_MINUS_PROB);
  Serial.print(F(",\"counterbalance\":"));
  Serial.print(PAV_COUNTERBALANCE ? F("true") : F("false"));
  Serial.println('}');

  Serial.print(F("{\"level\":\"000\",\"cs_plus_freq\":"));
  Serial.print(PAV_CS_PLUS_FREQ);
  Serial.print(F(",\"cs_minus_freq\":"));
  Serial.print(PAV_CS_MINUS_FREQ);
  Serial.print(F(",\"cue_duration\":"));
  Serial.print(PAV_CUE_DURATION);
  Serial.print(F(",\"trace_interval\":"));
  Serial.print(PAV_TRACE_INTERVAL);
  Serial.print(F(",\"consumption_ms\":"));
  Serial.print(PAV_CONSUMPTION_MS);
  Serial.print(F(",\"iti_mean\":"));
  Serial.print(PAV_ITI_MEAN);
  Serial.print(F(",\"iti_min\":"));
  Serial.print(PAV_ITI_MIN);
  Serial.print(F(",\"iti_max\":"));
  Serial.print(PAV_ITI_MAX);
  Serial.println('}');

  // Per-device config with armed status
  reportDeviceConfig(F("CUE"), cue.Armed(), PAV_CS_PLUS_FREQ, (uint32_t)PAV_CUE_DURATION);
  reportDeviceConfig(F("CUE2"), cue2.Armed(), PAV_CS_MINUS_FREQ, cue2.Duration());
  reportDeviceConfig(F("PUMP"), pump.Armed(), pump.Duration());
  reportDeviceConfig(F("PUMP2"), pump2.Armed(), pump2.Duration());
  reportDeviceConfig(F("LICK"), lickCircuit.Armed());
  reportDeviceConfig(F("MICROSCOPE"), microscope.Armed());
  reportDeviceLever(F("LEVER_RH"), rLever.Armed(), rLever.IsReinforced());
  reportDeviceLever(F("LEVER_LH"), lLever.Armed(), lLever.IsReinforced());
}

/// @brief End a session: trigger microscope, shut down scheduler, emit end event.
void EndSession() {
  SESSION_END_TIMESTAMP = millis();
  microscope.Trigger();
  scheduler.EndSession(SESSION_END_TIMESTAMP);

  Serial.print(F("{\"level\":\"007\",\"device\":\"CONTROLLER\",\"event\":\"END\",\"timestamp\":"));
  Serial.print(SESSION_END_TIMESTAMP - SESSION_START_TIMESTAMP);
  Serial.println('}');
}
