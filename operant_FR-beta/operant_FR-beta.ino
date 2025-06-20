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

uint32_t SESSION_START_TIMESTAMP;
uint32_t SESSION_END_TIMESTAMP;

void setup() {
  const uint32_t baudrate = 115200;
  
  delay(100);
  Serial.begin(baudrate);
  delay(100);

  StaticJsonDocument<128> doc;

  doc["level"] = F("000");
  doc["sketch"] = F("operant_FR.ino");
  doc["version"] = F("v1.1.1");
  doc["baud_rate"] = baudrate;

  JsonObject pins = doc.createNestedObject("pins");

  pins["rh_lever"] = rLever.Pin();
  pins["lh_lever"] = lLever.Pin();
  pins["cue"] = cue.Pin();
  pins["pump"] = pump.Pin();
  pins["lick_circuit"] = lickCircuit.Pin();
  pins["laser"] = laser.Pin();
  pins["microscope_trigger"] = microscope.TriggerPin();
  pins["microscope_timestamp"] = microscope.TimestampPin();

  serializeJson(doc, Serial);
  Serial.println();
  
  cue.Jingle();

  rLever.SetCue(&cue);
  rLever.SetPump(&pump);
  rLever.SetLaser(&laser);
  rLever.SetTimeoutIntervalLength(cue.Duration() + pump.Duration());
  rLever.SetReinforcement(true);
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
    StaticJsonDocument<128> doc;
    String cmd = Serial.readStringUntil('\n');
    DeserializationError error = deserializeJson(doc, cmd);

    if (error) {
      doc.clear();
      doc["level"] = 666;
      doc["desc"] = error.f_str();
      serializeJson(doc, Serial);
      Serial.println();
      return;
    }

    if (!doc["cmd"].isNull()) {
      int command = doc["cmd"];
      switch (command) {
        case 1001: rLever.ArmToggle(true); break;
        case 1000: rLever.ArmToggle(false); break;
        case 1074: rLever.SetTimeoutIntervalLength(doc["timeout"]); break;
        case 1081: rLever.SetReinforcement(true); break;
        case 1080: rLever.SetReinforcement(false); break;
        case 1301: lLever.ArmToggle(true); break;
        case 1300: lLever.ArmToggle(false); break;
        case 1374: lLever.SetTimeoutIntervalLength(doc["timeout"]); break;
        case 1381: lLever.SetReinforcement(true); break;
        case 1380: lLever.SetReinforcement(false); break;
        case 301: cue.ArmToggle(true); break;
        case 300: cue.ArmToggle(false); break;
        case 371: cue.SetFrequency(doc["frequency"]); break;
        case 372: cue.SetDuration(doc["duration"]); break;
        case 373: cue.SetTraceInterval(doc["trace"]); break;
        case 401: pump.ArmToggle(true); break;
        case 400: pump.ArmToggle(false); break;
        case 472: pump.SetDuration(doc["duration"]); break;
        case 473: pump.SetTraceInterval(doc["trace"]); break;
        case 501: lickCircuit.ArmToggle(true); break;
        case 500: lickCircuit.ArmToggle(false); break;
        case 601: laser.ArmToggle(true); break;
        case 600: laser.ArmToggle(false); break;
        case 671: laser.SetFrequency(doc["frequency"]); break;
        case 672: laser.SetDuration(doc["duration"]); break;
        case 673: laser.SetTraceInterval(doc["trace"]); break;
        case 681: laser.SetMode(true); break;
        case 682: laser.SetMode(false); break;
        case 901: microscope.ArmToggle(true); break;
        case 900: microscope.ArmToggle(false); break;
        case 101: StartSession(); SetDeviceTimestampOffset(SESSION_START_TIMESTAMP); break;
        case 100: EndSession(); ArmToggleDevices(false); break;
      }
    }
  }
}

void StartSession() {
  JsonDocument doc;
  SESSION_START_TIMESTAMP = millis();
 
  doc["level"] = 777;
  doc["device"] = F("CONTROLLER");
  doc["event"] = F("START");
  doc["ts"] = 0;

  // FIXME: output device setting here -> make a function for this?

  serializeJson(doc, Serial);
  Serial.println(); 

  OutputDeviceConfig();
}

void EndSession() {
  JsonDocument doc;
  SESSION_END_TIMESTAMP = millis();  

  doc["level"] = 777;
  doc["device"] = F("CONTROLLER");
  doc["event"] = F("END");
  doc["ts"] = SESSION_END_TIMESTAMP - SESSION_START_TIMESTAMP;

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

void OutputDeviceConfig() {
  JsonDocument doc;
 
  doc["level"] = 333;

  rLever.Config(&doc);
  lLever.Config(&doc);
  cue.Config(&doc);
  pump.Config(&doc);
  lickCircuit.Config(&doc);
  laser.Config(&doc);
  microscope.Config(&doc);

  serializeJson(doc, Serial);
  Serial.println();   
}
