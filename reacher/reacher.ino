#include <Arduino.h>
#include <ArduinoJson.h>

#include "Pins.h"
#include "Device.h"
#include "SwitchLever.h"
#include "LickCircuit.h"
#include "Cue.h"
#include "Pump.h"
#include "Laser.h"
#include "Microscope.h"
#include "Scheduler.h"
#include "Config.h"

// Paradigm selection
enum Paradigm : uint8_t {
  FIXED_RATIO,
  PROGRESSIVE_RATIO,
  OMISSION,
  VARIABLE_INTERVAL
};

Paradigm currentParadigm      = FIXED_RATIO;
uint8_t  PR_STEP              = 1;
uint32_t OMISSION_INTERVAL    = 20000;
uint32_t VI_INTERVAL          = 15000;

// Configurable parameters (updated via serial commands)
uint32_t CUE_DURATION       = DEFAULT_CUE_DURATION;
uint32_t CUE_FREQUENCY      = DEFAULT_CUE_FREQUENCY;
uint32_t PUMP_DURATION      = DEFAULT_PUMP_DURATION;
uint8_t  LASER_FREQUENCY    = DEFAULT_LASER_FREQUENCY;
uint32_t LASER_DURATION     = DEFAULT_LASER_DURATION;
uint32_t TIMEOUT_INTERVAL   = DEFAULT_TIMEOUT_INTERVAL;

// Device instances
SwitchLever rLever(PIN_LEVER_RH, "RH", DeviceType::LEVER_RH);
SwitchLever lLever(PIN_LEVER_LH, "LH", DeviceType::LEVER_LH);
SwitchLever* activeLever = &rLever;

Cue         cue(PIN_CUE, CUE_FREQUENCY, CUE_DURATION);
Pump        pump(PIN_PUMP, PUMP_DURATION);
LickCircuit lickCircuit(PIN_LICK_CIRCUIT);
Laser       laser(PIN_LASER, LASER_FREQUENCY, LASER_DURATION);
Microscope  microscope(PIN_MICROSCOPE_TRIG, PIN_MICROSCOPE_TS);

// Central scheduler
Scheduler scheduler;

// Session timestamps
uint32_t SESSION_START_TIMESTAMP;
uint32_t SESSION_END_TIMESTAMP;

// Forward declarations
void ParseCommands();
void StartSession();
void EndSession();
void SetDeviceTimestampOffset(uint32_t ts);
void ArmToggleDevices(bool toggle);
void ReconfigureChain();
const __FlashStringHelper* paradigmName(Paradigm p);

// Callbacks for input devices -> scheduler
void onLeverPress(DeviceType source, uint32_t timestamp) {
  scheduler.OnInputEvent(source, timestamp);
}

void onLeverRelease(DeviceType source) {
  scheduler.OnInputRelease(source);
}

void setup() {
  const uint32_t baudrate = 115200;
  JsonDocument setupJson;

  delay(100);
  Serial.begin(baudrate);
  delay(100);

  cue.Jingle();

  // Register devices with scheduler
  scheduler.RegisterLever(&rLever, DeviceType::LEVER_RH);
  scheduler.RegisterLever(&lLever, DeviceType::LEVER_LH);
  scheduler.RegisterLickCircuit(&lickCircuit);
  scheduler.RegisterCue(&cue);
  scheduler.RegisterPump(&pump);
  scheduler.RegisterLaser(&laser);
  scheduler.RegisterMicroscope(&microscope);

  // Set input callbacks
  rLever.SetCallback(onLeverPress);
  rLever.SetReleaseCallback(onLeverRelease);
  lLever.SetCallback(onLeverPress);
  lLever.SetReleaseCallback(onLeverRelease);

  // Configure lever roles
  rLever.SetActiveLever(true);
  lLever.SetActiveLever(false);

  // Configure default paradigm: FR1
  scheduler.SetTimeoutInterval(TIMEOUT_INTERVAL);
  configureFixedRatio(scheduler, cue, pump, laser, 1);

  // Send setup JSON
  setupJson[F("level")] = F("000");
  setupJson[F("device")] = F("CONTROLLER");
  setupJson[F("sketch")] = F("reacher.ino");
  setupJson[F("version")] = F("v2.0.0");
  setupJson[F("baud_rate")] = baudrate;
  setupJson[F("schedule")] = paradigmName(currentParadigm);

  serializeJson(setupJson, Serial);
  Serial.println();
}

void loop() {
  uint32_t currentTimestamp = millis();

  // Poll inputs
  rLever.Monitor(currentTimestamp);
  lLever.Monitor(currentTimestamp);
  lickCircuit.Monitor(currentTimestamp);

  // Scheduler: tick triggers, pending actions, output devices
  scheduler.Update(currentTimestamp);

  // Microscope frame handling
  microscope.HandleFrameSignal();

  // Process serial commands
  ParseCommands();
}

void ParseCommands() {
  if (Serial.available() > 0) {
    JsonDocument inputJson;
    String cmd = Serial.readStringUntil('\n');
    DeserializationError error = deserializeJson(inputJson, cmd);

    if (error) {
      inputJson.clear();
      inputJson["level"] = "006";
      inputJson["desc"] = error.f_str();

      serializeJson(inputJson, Serial);
      Serial.println();
      while (Serial.available() > 0) Serial.read();
      return;
    }

    if (!inputJson["cmd"].isNull()) {
      int command = inputJson["cmd"];
      switch (command) {

        // RH lever commands
        case 1001: rLever.ArmToggle(true); break;
        case 1000: rLever.ArmToggle(false); break;
        case 1074: TIMEOUT_INTERVAL = inputJson["timeout"]; scheduler.SetTimeoutInterval(TIMEOUT_INTERVAL); break;
        case 1075: scheduler.SetRatio(inputJson["ratio"]); break;
        case 1081: rLever.SetActiveLever(true); activeLever = &rLever; break;
        case 1080: rLever.SetActiveLever(false); break;

        // LH lever commands
        case 1301: lLever.ArmToggle(true); break;
        case 1300: lLever.ArmToggle(false); break;
        case 1374: TIMEOUT_INTERVAL = inputJson["timeout"]; scheduler.SetTimeoutInterval(TIMEOUT_INTERVAL); break;
        case 1375: scheduler.SetRatio(inputJson["ratio"]); break;
        case 1381: lLever.SetActiveLever(true); activeLever = &lLever; break;
        case 1380: lLever.SetActiveLever(false); break;

        // Cue commands
        case 301: cue.ArmToggle(true); break;
        case 300: cue.ArmToggle(false); break;
        case 371: CUE_FREQUENCY = inputJson["frequency"]; cue.SetFrequency(CUE_FREQUENCY); break;
        case 372: CUE_DURATION = inputJson["duration"]; cue.SetDuration(CUE_DURATION); ReconfigureChain(); break;
        case 373: /* trace interval now encoded in chain offsets */ break;

        // Pump commands
        case 401: pump.ArmToggle(true); break;
        case 400: pump.ArmToggle(false); break;
        case 472: PUMP_DURATION = inputJson["duration"]; pump.SetDuration(PUMP_DURATION); ReconfigureChain(); break;
        case 473: /* trace interval now encoded in chain offsets */ break;

        // Lick circuit commands
        case 501: lickCircuit.ArmToggle(true); break;
        case 500: lickCircuit.ArmToggle(false); break;

        // Laser commands
        case 601: laser.ArmToggle(true); break;
        case 600: laser.ArmToggle(false); break;
        case 603: laser.Test(millis()); break;
        case 671: LASER_FREQUENCY = inputJson["frequency"]; laser.SetFrequency(LASER_FREQUENCY); break;
        case 672: LASER_DURATION = inputJson["duration"]; laser.SetDuration(LASER_DURATION); ReconfigureChain(); break;
        case 673: /* trace interval now encoded in chain offsets */ break;
        case 681: laser.SetMode(true); break;   // CONTINGENT
        case 682: laser.SetMode(false); break;  // INDEPENDENT

        // Microscope commands
        case 901: microscope.ArmToggle(true); break;
        case 900: microscope.ArmToggle(false); break;

        // Session setup commands
        case 201: scheduler.SetRatio(inputJson["ratio"]); break;
        case 202: {
          uint8_t p = inputJson["paradigm"];
          if (p <= 3) {
            currentParadigm = static_cast<Paradigm>(p);
            if (currentParadigm == OMISSION) {
              TIMEOUT_INTERVAL = 0;
              scheduler.SetTimeoutInterval(0);
            }
            ReconfigureChain();
          }
          break;
        }
        case 203: OMISSION_INTERVAL = inputJson["interval"]; if (currentParadigm == OMISSION) ReconfigureChain(); break;
        case 204: VI_INTERVAL = inputJson["interval"]; if (currentParadigm == VARIABLE_INTERVAL) ReconfigureChain(); break;
        case 205: {
          PR_STEP = inputJson["step"];
          if (currentParadigm == PROGRESSIVE_RATIO) {
            Trigger* t = scheduler.GetTrigger(0);
            if (t) t->prStep = PR_STEP;
          }
          break;
        }

        // Controller commands
        case 101: StartSession(); SetDeviceTimestampOffset(SESSION_START_TIMESTAMP); break;
        case 100: EndSession(); ArmToggleDevices(false); break;

        // Error
        default: {
          JsonDocument doc;
      doc["level"] = F("006");
          doc["desc"] = F("Command not found");
          serializeJson(doc, Serial);
          Serial.println();
        }
      }
    }
  }
}

const __FlashStringHelper* paradigmName(Paradigm p) {
  switch (p) {
    case FIXED_RATIO:        return F("FIXED_RATIO");
    case PROGRESSIVE_RATIO:  return F("PROGRESSIVE_RATIO");
    case OMISSION:           return F("OMISSION");
    case VARIABLE_INTERVAL:  return F("VARIABLE_INTERVAL");
    default:                 return F("UNKNOWN");
  }
}

void ReconfigureChain() {
  Trigger* t = scheduler.GetTrigger(0);
  uint8_t currentRatio = t ? t->threshold : 1;

  switch (currentParadigm) {
    case FIXED_RATIO:
      configureFixedRatio(scheduler, cue, pump, laser, currentRatio);
      break;
    case PROGRESSIVE_RATIO:
      configureProgressiveRatio(scheduler, cue, pump, laser, currentRatio, PR_STEP);
      break;
    case OMISSION:
      configureOmission(scheduler, cue, pump, laser, OMISSION_INTERVAL);
      break;
    case VARIABLE_INTERVAL:
      configureVariableInterval(scheduler, cue, pump, laser, VI_INTERVAL);
      break;
  }
}

void StartSession() {
  SESSION_START_TIMESTAMP = millis();
  microscope.Trigger();
  scheduler.StartSession(SESSION_START_TIMESTAMP);

  {
    JsonDocument doc;
    doc[F("level")] = F("007");
    doc[F("device")] = F("CONTROLLER");
    doc[F("event")] = F("START");
    doc["timestamp"] = 0;

    serializeJson(doc, Serial);
    Serial.println();
  }

  // Send current settings
  JsonDocument settings;
  settings[F("level")] = F("000");
  settings[F("device")] = F("NA");
  JsonObject cueObj = settings.createNestedObject(F("cue"));
  JsonObject pumpObj = settings.createNestedObject(F("pump"));
  JsonObject laserObj = settings.createNestedObject(F("laser"));
  JsonObject leverObj = settings.createNestedObject(F("active_lever"));

  cueObj[F("frequency")] = CUE_FREQUENCY;
  cueObj[F("duration")] = CUE_DURATION;
  cueObj[F("trace")] = 0;

  pumpObj[F("duration")] = PUMP_DURATION;
  pumpObj[F("trace")] = CUE_DURATION;

  laserObj[F("frequency")] = LASER_FREQUENCY;
  laserObj[F("duration")] = LASER_DURATION;
  laserObj[F("trace")] = CUE_DURATION;

  leverObj[F("timeout")] = TIMEOUT_INTERVAL;

  settings[F("paradigm")] = paradigmName(currentParadigm);
  switch (currentParadigm) {
    case PROGRESSIVE_RATIO:  settings[F("pr_step")] = PR_STEP; break;
    case OMISSION:           settings[F("omission_interval")] = OMISSION_INTERVAL; break;
    case VARIABLE_INTERVAL:  settings[F("variable_interval")] = VI_INTERVAL; break;
    default: break;
  }

  serializeJson(settings, Serial);
  Serial.println();
}

void EndSession() {
  SESSION_END_TIMESTAMP = millis();
  microscope.Trigger();
  scheduler.EndSession(SESSION_END_TIMESTAMP);

  JsonDocument doc;
  doc[F("level")] = F("007");
  doc[F("device")] = F("CONTROLLER");
  doc[F("event")] = F("END");
  doc["timestamp"] = SESSION_END_TIMESTAMP - SESSION_START_TIMESTAMP;

  serializeJson(doc, Serial);
  Serial.println();
}

void SetDeviceTimestampOffset(uint32_t ts) {
  rLever.SetOffset(ts);
  lLever.SetOffset(ts);
  cue.SetOffset(ts);
  pump.SetOffset(ts);
  lickCircuit.SetOffset(ts);
  laser.SetOffset(ts);
  microscope.SetOffset(ts);
}

void ArmToggleDevices(bool toggle) {
  rLever.ArmToggle(toggle);
  lLever.ArmToggle(toggle);
  cue.ArmToggle(toggle);
  pump.ArmToggle(toggle);
  lickCircuit.ArmToggle(toggle);
  laser.ArmToggle(toggle);
  microscope.ArmToggle(toggle);
}
