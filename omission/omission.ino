/**
 * @file omission.ino
 * @brief REACHER v2.0 Omission operant conditioning controller.
 *
 * Reward fires after the animal withholds lever pressing for a configurable
 * absence interval. Any press resets the timer. No timeout is applied.
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
uint32_t OMISSION_INTERVAL  = 20000;

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
  Serial.println(F("{\"level\":\"000\",\"device\":\"CONTROLLER\",\"sketch\":\"omission.ino\",\"version\":\"v2.0.0\",\"baud_rate\":115200,\"schedule\":\"OMISSION\"}"));
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

  // Omission has no timeout
  scheduler.SetTimeoutInterval(0);
  configureOmission(scheduler, cue, pump, laser, OMISSION_INTERVAL);

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
  configureOmission(scheduler, cue, pump, laser, OMISSION_INTERVAL);
}

void StartSession() {
  SESSION_START_TIMESTAMP = millis();
  microscope.Trigger();
  scheduler.StartSession(SESSION_START_TIMESTAMP);

  Serial.println(F("{\"level\":\"007\",\"device\":\"CONTROLLER\",\"event\":\"START\",\"timestamp\":0}"));

  Serial.print(F("{\"level\":\"000\",\"device\":\"CONTROLLER\",\"paradigm\":\"OMISSION\",\"active_lever\":\""));
  Serial.print((activeLever == &rLever) ? F("RH") : F("LH"));
  Serial.println(F("\"}"));

  reportDeviceConfig(F("CUE"), cue.Armed(), CUE_FREQUENCY, CUE_DURATION);
  reportDeviceConfig(F("CUE2"), cue2.Armed(), cue2.Frequency(), cue2.Duration());
  reportDeviceConfig(F("PUMP"), pump.Armed(), PUMP_DURATION);
  reportDeviceConfig(F("PUMP2"), pump2.Armed(), pump2.Duration());
  reportDeviceConfig(F("LASER"), laser.Armed(), LASER_FREQUENCY, LASER_DURATION);
  reportDeviceConfig(F("LICK"), lickCircuit.Armed());
  reportDeviceConfig(F("MICROSCOPE"), microscope.Armed());
  reportDeviceLever(F("LEVER_RH"), rLever.Armed(), rLever.IsReinforced());
  reportDeviceLever(F("LEVER_LH"), lLever.Armed(), lLever.IsReinforced());

  Serial.print(F("{\"level\":\"000\",\"omission_interval\":"));
  Serial.print(OMISSION_INTERVAL);
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
      switch (command) {

        // RH lever commands
        case Cmd::LEVER_RH_ARM:          rLever.ArmToggle(true); break;
        case Cmd::LEVER_RH_DISARM:       rLever.ArmToggle(false); break;
        case Cmd::LEVER_RH_SET_ACTIVE:
          rLever.SetActiveLever(true); activeLever = &rLever;
          logParamChange(F("LEVER_RH"), F("reinforced"), true); break;
        case Cmd::LEVER_RH_SET_INACTIVE:
          rLever.SetActiveLever(false);
          logParamChange(F("LEVER_RH"), F("reinforced"), false); break;

        // LH lever commands
        case Cmd::LEVER_LH_ARM:          lLever.ArmToggle(true); break;
        case Cmd::LEVER_LH_DISARM:       lLever.ArmToggle(false); break;
        case Cmd::LEVER_LH_SET_ACTIVE:
          lLever.SetActiveLever(true); activeLever = &lLever;
          logParamChange(F("LEVER_LH"), F("reinforced"), true); break;
        case Cmd::LEVER_LH_SET_INACTIVE:
          lLever.SetActiveLever(false);
          logParamChange(F("LEVER_LH"), F("reinforced"), false); break;

        // Cue commands
        case Cmd::CUE_ARM:            cue.ArmToggle(true); break;
        case Cmd::CUE_DISARM:         cue.ArmToggle(false); break;
        case Cmd::CUE_SET_FREQUENCY:
          CUE_FREQUENCY = inputJson["frequency"]; cue.SetFrequency(CUE_FREQUENCY);
          logParamChange(F("CUE"), F("frequency"), CUE_FREQUENCY); break;
        case Cmd::CUE_SET_DURATION:
          CUE_DURATION = inputJson["duration"]; cue.SetDuration(CUE_DURATION); ReconfigureChain();
          logParamChange(F("CUE"), F("duration"), CUE_DURATION); break;
        case Cmd::CUE_TEST:
          cue.Test(millis());
          logParamChange(F("CUE"), F("test"), F("FIRED")); break;

        // Secondary cue commands
        case Cmd::CUE2_ARM:           cue2.ArmToggle(true); break;
        case Cmd::CUE2_DISARM:        cue2.ArmToggle(false); break;
        case Cmd::CUE2_SET_FREQUENCY:
          cue2.SetFrequency(inputJson["frequency"]);
          logParamChange(F("CUE2"), F("frequency"), (uint32_t)inputJson["frequency"]); break;
        case Cmd::CUE2_TEST:
          cue2.Test(millis());
          logParamChange(F("CUE2"), F("test"), F("FIRED")); break;
        case Cmd::CUE2_SET_DURATION:
          cue2.SetDuration(inputJson["duration"]);
          logParamChange(F("CUE2"), F("duration"), (uint32_t)inputJson["duration"]); break;

        // Pump commands
        case Cmd::PUMP_ARM:           pump.ArmToggle(true); break;
        case Cmd::PUMP_DISARM:        pump.ArmToggle(false); break;
        case Cmd::PUMP_SET_DURATION:
          PUMP_DURATION = inputJson["duration"]; pump.SetDuration(PUMP_DURATION); ReconfigureChain();
          logParamChange(F("PUMP"), F("duration"), PUMP_DURATION); break;
        case Cmd::PUMP_TEST:
          pump.Test(millis());
          logParamChange(F("PUMP"), F("test"), F("FIRED")); break;

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

        // Laser commands
        case Cmd::LASER_ARM:              laser.ArmToggle(true); break;
        case Cmd::LASER_DISARM:           laser.ArmToggle(false); break;
        case Cmd::LASER_TEST:
          laser.Test(millis());
          logParamChange(F("LASER"), F("test"), F("FIRED")); break;
        case Cmd::LASER_SET_FREQUENCY:
          LASER_FREQUENCY = inputJson["frequency"]; laser.SetFrequency(LASER_FREQUENCY);
          logParamChange(F("LASER"), F("frequency"), (uint32_t)LASER_FREQUENCY); break;
        case Cmd::LASER_SET_DURATION:
          LASER_DURATION = inputJson["duration"]; laser.SetDuration(LASER_DURATION); ReconfigureChain();
          logParamChange(F("LASER"), F("duration"), LASER_DURATION); break;
        case Cmd::LASER_MODE_CONTINGENT:
          laser.SetMode(true);
          logParamChange(F("LASER"), F("mode"), F("CONTINGENT")); break;
        case Cmd::LASER_MODE_INDEPENDENT:
          laser.SetMode(false);
          logParamChange(F("LASER"), F("mode"), F("INDEPENDENT")); break;

        // Microscope commands
        case Cmd::MICROSCOPE_ARM:    microscope.ArmToggle(true); break;
        case Cmd::MICROSCOPE_DISARM: microscope.ArmToggle(false); break;
        case Cmd::MICROSCOPE_TEST:
          microscope.Trigger();
          logParamChange(F("MICROSCOPE"), F("test"), F("FIRED")); break;

        // Session setup commands
        case Cmd::SET_OMISSION_INTERVAL:
          OMISSION_INTERVAL = inputJson["interval"]; ReconfigureChain();
          logParamChange(F("CONTROLLER"), F("omission_interval"), OMISSION_INTERVAL); break;

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
