/**
 * @file pr.ino
 * @brief REACHER v2.0 Progressive Ratio (PR) operant conditioning controller.
 *
 * Like FR, but the press threshold increases by a configurable step after each
 * reward. Uses arithmetic progression (not Richardson & Roberts exponential).
 *
 * **Baud rate:** 115200
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
#include <Laser.h>
#include <Microscope.h>
#include <Scheduler.h>
#include <ReacherHelpers.h>
#include "Config.h"

// Configurable parameters
uint32_t CUE_DURATION       = DEFAULT_CUE_DURATION;
uint32_t CUE_FREQUENCY      = DEFAULT_CUE_FREQUENCY;
uint32_t PUMP_DURATION      = DEFAULT_PUMP_DURATION;
uint8_t  LASER_FREQUENCY    = DEFAULT_LASER_FREQUENCY;
uint32_t LASER_DURATION     = DEFAULT_LASER_DURATION;
uint32_t TIMEOUT_INTERVAL   = DEFAULT_TIMEOUT_INTERVAL;
uint32_t TRACE_INTERVAL     = 0;
uint8_t  PR_STEP            = 1;

// Device instances
SwitchLever rLever(PIN_LEVER_RH, "RH", DeviceType::LEVER_RH);
SwitchLever lLever(PIN_LEVER_LH, "LH", DeviceType::LEVER_LH);
SwitchLever* activeLever = &rLever;

Cue         cue(PIN_CUE, CUE_FREQUENCY, CUE_DURATION);
Cue         cue2(PIN_CUE_2, CUE_FREQUENCY, CUE_DURATION);
Pump        pump(PIN_PUMP, PUMP_DURATION);
Pump        pump2(PIN_PUMP_2, PUMP_DURATION);
LickCircuit lickCircuit(PIN_LICK_CIRCUIT);
Laser       laser(PIN_LASER, LASER_FREQUENCY, LASER_DURATION);
Microscope  microscope(PIN_MICROSCOPE_TRIG, PIN_MICROSCOPE_TS);

Scheduler scheduler;

DeviceSet devices = { &rLever, &lLever, &cue, &cue2, &pump, &pump2, &lickCircuit, &laser, &microscope };

uint32_t SESSION_START_TIMESTAMP;
uint32_t SESSION_END_TIMESTAMP;

// Forward declarations
void ParseCommands();
void StartSession();
void EndSession();
void ReconfigureChain();
void SendIdentification();

void onLeverPress(DeviceType source, uint32_t timestamp) {
  scheduler.OnInputEvent(source, timestamp);
}

void onLeverRelease(DeviceType source) {
  scheduler.OnInputRelease(source);
}

void SendIdentification() {
  Serial.println(F("{\"level\":\"000\",\"device\":\"CONTROLLER\",\"sketch\":\"pr.ino\",\"version\":\"v2.0.0\",\"baud_rate\":115200,\"schedule\":\"PROGRESSIVE_RATIO\"}"));
}

void setup() {
  delay(100);
  Serial.begin(115200);
  Serial.setTimeout(10);
  delay(100);

  cue.Jingle();

  scheduler.RegisterLever(&rLever, DeviceType::LEVER_RH);
  scheduler.RegisterLever(&lLever, DeviceType::LEVER_LH);
  scheduler.RegisterLickCircuit(&lickCircuit);
  scheduler.RegisterCue(&cue);
  scheduler.RegisterCue2(&cue2);
  scheduler.RegisterPump(&pump);
  scheduler.RegisterPump2(&pump2);
  scheduler.RegisterLaser(&laser);
  scheduler.RegisterMicroscope(&microscope);

  rLever.SetCallback(onLeverPress);
  rLever.SetReleaseCallback(onLeverRelease);
  lLever.SetCallback(onLeverPress);
  lLever.SetReleaseCallback(onLeverRelease);

  rLever.SetActiveLever(true);
  lLever.SetActiveLever(false);

  scheduler.SetTimeoutInterval(TIMEOUT_INTERVAL);
  configureProgressiveRatio(scheduler, cue, pump, laser, 1, PR_STEP, DeviceType::LEVER_RH, TRACE_INTERVAL);

  SendIdentification();
}

void loop() {
  uint32_t currentTimestamp = millis();

  rLever.Monitor(currentTimestamp);
  lLever.Monitor(currentTimestamp);
  lickCircuit.Monitor(currentTimestamp);

  scheduler.Update(currentTimestamp);
  microscope.HandleFrameSignal();
  ParseCommands();
}

void ReconfigureChain() {
  Trigger* t = scheduler.GetTrigger(0);
  uint8_t currentRatio = t ? t->threshold : 1;
  DeviceType timeoutTarget = (activeLever == &rLever) ? DeviceType::LEVER_RH : DeviceType::LEVER_LH;
  configureProgressiveRatio(scheduler, cue, pump, laser, currentRatio, PR_STEP, timeoutTarget, TRACE_INTERVAL);
}

void StartSession() {
  SESSION_START_TIMESTAMP = millis();
  microscope.Trigger();
  scheduler.StartSession(SESSION_START_TIMESTAMP);

  Serial.println(F("{\"level\":\"007\",\"device\":\"CONTROLLER\",\"event\":\"START\",\"timestamp\":0}"));

  Serial.print(F("{\"level\":\"000\",\"device\":\"CONTROLLER\",\"paradigm\":\"PROGRESSIVE_RATIO\",\"timeout\":"));
  Serial.print(TIMEOUT_INTERVAL);
  Serial.print(F(",\"active_lever\":\""));
  Serial.print((activeLever == &rLever) ? F("RH") : F("LH"));
  Serial.print(F("\",\"trace_interval\":"));
  Serial.print(TRACE_INTERVAL);
  Serial.println('}');

  reportDeviceConfig(F("CUE"), cue.Armed(), CUE_FREQUENCY, CUE_DURATION);
  reportDeviceConfig(F("CUE2"), cue2.Armed(), cue2.Frequency(), cue2.Duration());
  reportDeviceConfig(F("PUMP"), pump.Armed(), PUMP_DURATION);
  reportDeviceConfig(F("PUMP2"), pump2.Armed(), pump2.Duration());
  reportDeviceConfig(F("LASER"), laser.Armed(), LASER_FREQUENCY, LASER_DURATION);
  reportDeviceConfig(F("LICK"), lickCircuit.Armed());
  reportDeviceConfig(F("MICROSCOPE"), microscope.Armed());
  reportDeviceLever(F("LEVER_RH"), rLever.Armed(), rLever.IsReinforced());
  reportDeviceLever(F("LEVER_LH"), lLever.Armed(), lLever.IsReinforced());

  Serial.print(F("{\"level\":\"000\",\"pr_step\":"));
  Serial.print(PR_STEP);
  Serial.println('}');
}

void EndSession() {
  SESSION_END_TIMESTAMP = millis();
  microscope.Trigger();
  scheduler.EndSession(SESSION_END_TIMESTAMP);

  Serial.print(F("{\"level\":\"007\",\"device\":\"CONTROLLER\",\"event\":\"END\",\"timestamp\":"));
  Serial.print(SESSION_END_TIMESTAMP - SESSION_START_TIMESTAMP);
  Serial.println('}');
}

void ParseCommands() {
  if (Serial.available() > 0) {
    JsonDocument inputJson;
    char buf[128];
    int len = Serial.readBytesUntil('\n', buf, sizeof(buf) - 1);
    buf[len] = '\0';

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

      if (handleCommonDeviceCommand(devices, command, inputJson)) {
        // Update local shadow variables and reconfigure chain for duration changes
        switch (command) {
          case Cmd::CUE_SET_FREQUENCY:   CUE_FREQUENCY = inputJson["frequency"]; break;
          case Cmd::CUE_SET_DURATION:    CUE_DURATION = inputJson["duration"]; ReconfigureChain(); break;
          case Cmd::PUMP_SET_DURATION:   PUMP_DURATION = inputJson["duration"]; ReconfigureChain(); break;
          case Cmd::LASER_SET_FREQUENCY: LASER_FREQUENCY = inputJson["frequency"]; break;
          case Cmd::LASER_SET_DURATION:  LASER_DURATION = inputJson["duration"]; ReconfigureChain(); break;
        }
      } else {
        switch (command) {
          // RH lever commands
          case Cmd::LEVER_RH_ARM:          rLever.ArmToggle(true); break;
          case Cmd::LEVER_RH_DISARM:       rLever.ArmToggle(false); break;
          case Cmd::LEVER_RH_SET_TIMEOUT:
            TIMEOUT_INTERVAL = inputJson["timeout"]; scheduler.SetTimeoutInterval(TIMEOUT_INTERVAL);
            logParamChange(F("LEVER_RH"), F("timeout"), TIMEOUT_INTERVAL); break;
          case Cmd::LEVER_RH_SET_RATIO:
            scheduler.SetRatio(inputJson["ratio"]);
            logParamChange(F("LEVER_RH"), F("ratio"), (uint32_t)inputJson["ratio"]); break;
          case Cmd::LEVER_RH_SET_ACTIVE:
            rLever.SetActiveLever(true); activeLever = &rLever;
            logParamChange(F("LEVER_RH"), F("reinforced"), true); break;
          case Cmd::LEVER_RH_SET_INACTIVE:
            rLever.SetActiveLever(false);
            logParamChange(F("LEVER_RH"), F("reinforced"), false); break;

          // LH lever commands
          case Cmd::LEVER_LH_ARM:          lLever.ArmToggle(true); break;
          case Cmd::LEVER_LH_DISARM:       lLever.ArmToggle(false); break;
          case Cmd::LEVER_LH_SET_TIMEOUT:
            TIMEOUT_INTERVAL = inputJson["timeout"]; scheduler.SetTimeoutInterval(TIMEOUT_INTERVAL);
            logParamChange(F("LEVER_LH"), F("timeout"), TIMEOUT_INTERVAL); break;
          case Cmd::LEVER_LH_SET_RATIO:
            scheduler.SetRatio(inputJson["ratio"]);
            logParamChange(F("LEVER_LH"), F("ratio"), (uint32_t)inputJson["ratio"]); break;
          case Cmd::LEVER_LH_SET_ACTIVE:
            lLever.SetActiveLever(true); activeLever = &lLever;
            logParamChange(F("LEVER_LH"), F("reinforced"), true); break;
          case Cmd::LEVER_LH_SET_INACTIVE:
            lLever.SetActiveLever(false);
            logParamChange(F("LEVER_LH"), F("reinforced"), false); break;

          // Session setup commands
          case Cmd::SET_RATIO:
            scheduler.SetRatio(inputJson["ratio"]);
            logParamChange(F("CONTROLLER"), F("ratio"), (uint32_t)inputJson["ratio"]); break;
          case Cmd::SET_PR_STEP: {
            PR_STEP = inputJson["step"];
            Trigger* t = scheduler.GetTrigger(0);
            if (t) t->prStep = PR_STEP;
            logParamChange(F("CONTROLLER"), F("pr_step"), (uint32_t)PR_STEP);
            break;
          }
          case Cmd::SET_TRACE_INTERVAL:
            TRACE_INTERVAL = inputJson["interval"];
            ReconfigureChain();
            logParamChange(F("CONTROLLER"), F("trace_interval"), TRACE_INTERVAL); break;

          // Controller commands
          case Cmd::SESSION_START:
            StartSession(); setDeviceTimestampOffset(devices, SESSION_START_TIMESTAMP); break;
          case Cmd::SESSION_END:
            EndSession(); armToggleDevices(devices, false); break;
          case Cmd::IDENTIFY:      SendIdentification(); break;
          case Cmd::TEST_CHAIN:
            scheduler.TestChain(millis());
            logParamChange(F("CONTROLLER"), F("test_chain"), F("FIRED")); break;
          case Cmd::TEST_MODE: {
            bool enable = inputJson["enable"] | false;
            scheduler.SetTestMode(enable, millis());
            if (!enable) ReconfigureChain();
            logParamChange(F("CONTROLLER"), F("test_mode"), scheduler.IsTestMode());
            break;
          }

          default:
            Serial.println(F("{\"level\":\"006\",\"desc\":\"Command not found\"}"));
            break;
        }
      }
    }
  }
}
