#include <Arduino.h>
#include <ArduinoJson.h>

#include "Device.h"
#include "SwitchLever.h"
#include "Cue.h"
#include "Pump.h"
#include "LickCircuit.h"
#include "Laser.h"
#include "Microscope.h"

SwitchLever rLever(10, "RH");
SwitchLever lLever(13, "LH");
Cue cue(3, 8000, 1600, 0);
Pump pump(4, 2000, cue.Duration());
LickCircuit lickCircuit(5);
Laser laser(6, 40, 5000, cue.Duration());
Microscope microscope(9, 2);

JsonDocument doc;

uint32_t SESSION_START_TIMESTAMP;
uint32_t SESSION_END_TIMESTAMP;

void setup() {
  const uint32_t baudrate = 115200;
  
  delay(100);
  Serial.begin(baudrate);
  delay(100);

  JsonDocument setupJson;
  setupJson["level"] = F("000");
  setupJson["sketch"] = F("operant_FR.ino");
  setupJson["version"] = F("v1.1.1");
  setupJson["baud_rate"] = baudrate;

  serializeJson(setupJson, Serial);
  Serial.println();
  
  cue.Jingle();

  rLever.SetCue(&cue);
  rLever.SetPump(&pump);
  rLever.SetLaser(&laser);
  rLever.SetTimeoutIntervalLength(cue.Duration() + pump.Duration());
  rLever.SetActiveLever(true);

  lLever.SetCue(&cue);
  lLever.SetPump(&pump);
  lLever.SetLaser(&laser);
  lLever.SetTimeoutIntervalLength(cue.Duration() + pump.Duration());
  lLever.SetActiveLever(false);
}

void loop() {
  uint32_t currentTimestamp = millis();
  
  rLever.Monitor(currentTimestamp);
  lLever.Monitor(currentTimestamp);
  lickCircuit.Monitor(currentTimestamp);
  cue.Await(currentTimestamp);
  pump.Await(currentTimestamp);
  laser.Await(currentTimestamp);
  microscope.HandleFrameSignal();
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
        case 1074: rLever.SetTimeoutIntervalLength(inputJson["timeout"]); break;
        case 1081: rLever.SetActiveLever(true); break;
        case 1080: rLever.SetActiveLever(false); break;

        // LH lever commands
        case 1301: lLever.ArmToggle(true); break;
        case 1300: lLever.ArmToggle(false); break;
        case 1374: lLever.SetTimeoutIntervalLength(inputJson["timeout"]); break;
        case 1381: lLever.SetActiveLever(true); break;
        case 1380: lLever.SetActiveLever(false); break;

        // cue commands
        case 301: cue.ArmToggle(true); break;
        case 300: cue.ArmToggle(false); break;
        case 371: cue.SetFrequency(inputJson["frequency"]); break;
        case 372: cue.SetDuration(inputJson["duration"]); break;
        case 373: cue.SetTraceInterval(inputJson["trace"]); break;

        // pump commands
        case 401: pump.ArmToggle(true); break;
        case 400: pump.ArmToggle(false); break;
        case 472: pump.SetDuration(inputJson["duration"]); break;
        case 473: pump.SetTraceInterval(inputJson["trace"]); break;

        // lick circuit commands
        case 501: lickCircuit.ArmToggle(true); break;
        case 500: lickCircuit.ArmToggle(false); break;

        // laser commands
        case 601: laser.ArmToggle(true); break;
        case 600: laser.ArmToggle(false); break;
        case 603: laser.Test(millis()); break;
        case 671: laser.SetFrequency(inputJson["frequency"]); break;
        case 672: laser.SetDuration(inputJson["duration"]); break;
        case 673: laser.SetTraceInterval(inputJson["trace"]); break;
        case 681: laser.SetMode(true); break;
        case 682: laser.SetMode(false); break;

        // microscope commands
        case 901: microscope.ArmToggle(true); break;
        case 900: microscope.ArmToggle(false); break;

        // controller commands
        case 101: StartSession(); SetDeviceTimestampOffset(SESSION_START_TIMESTAMP); break;
        case 100: EndSession(); ArmToggleDevices(false); break;

        // error
        default:
          JsonDocument doc;

          doc["level"] = F("006");
          doc["desc"] = F("Command not found");

          serializeJson(doc, Serial);
          Serial.println();
      }
    }
  }
}

void StartSession() {
  SESSION_START_TIMESTAMP = millis();

  doc.clear();
  doc[F("level")] = F("007");
  doc[F("device")] = F("CONTROLLER");
  doc[F("event")] = F("START");
  doc["timestamp"] = 0;

  serializeJson(doc, Serial);
  Serial.println(); 
}

void EndSession() {
  SESSION_END_TIMESTAMP = millis();  

  doc.clear();
  doc[F("level")] = F("007");
  doc[F("device")] = F("CONTROLLER");
  doc[F("event")] = F("END");
  doc["timestamp"] = SESSION_END_TIMESTAMP - SESSION_START_TIMESTAMP;

  // manually write LOW signals before shut off
  noTone(cue.Pin());
  digitalWrite(pump.Pin(), LOW);
  digitalWrite(laser.Pin(), LOW);

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
