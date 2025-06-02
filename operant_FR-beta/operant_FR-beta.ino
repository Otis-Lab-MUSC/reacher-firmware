#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "Device.h"
#include "SwitchLever.h"
#include "Cue.h"
#include "Pump.h"

#define SKETCH_NAME "operant_FR.ino"
#define VERSION "v1.0.1"
#define BAUDRATE 115200

#define RH_LEVER_PIN 10
#define LH_LEVER_PIN 13
#define CUE_PIN 3
#define PUMP_PIN 4

#define DEFAULT_CUE_FREQUENCY 8000
#define DEFAULT_CUE_DURATION 1600
#define DEFAULT_PUMP_DURATION 2000

SwitchLever rLever(RH_LEVER_PIN, "RH", true);
SwitchLever lLever(LH_LEVER_PIN, "LH", false);
Cue cue(CUE_PIN, DEFAULT_CUE_FREQUENCY, DEFAULT_CUE_DURATION);
Pump pump(PUMP_PIN, cue.Duration(), DEFAULT_PUMP_DURATION);

uint32_t SESSION_START_TIMESTAMP;
uint32_t SESSION_END_TIMESTAMP;

void setup() {
  Serial.begin(BAUDRATE);
  delay(100);
  
  rLever.SetCue(&cue);
  rLever.SetPump(&pump);

  cue.Jingle();
  
  Serial.println(F("========== SESSION START =========="));
  Serial.println();
}

void loop() {
  rLever.Monitor();
  lLever.Monitor();
  cue.Await();
  pump.Await();
  ParseCommands();
}

void ParseCommands() {
  if (Serial.available() > 0) {
    JsonDocument json;
    String cmd = Serial.readStringUntil('\n');
    String desc;
    DeserializationError error = deserializeJson(json, cmd);

    if (error) {
      desc = F("Command parsing failed: ");
      desc += error.f_str();
      
      json["level"] = F("error");
      json["desc"] = desc;

      serializeJsonPretty(json, Serial);
    } 
    
    else if(json["cmd"]) {

      /* RH Lever Commands */
      if (json["cmd"] == 1001) {
        rLever.ArmToggle(true);
      } 
      else if (json["cmd"] == 1000) {
        rLever.ArmToggle(false);
      }

      /* LH Lever Commands */
      else if (json["cmd"] == 1301) {
        lLever.ArmToggle(true);
      } 
      else if (json["cmd"] == 1300) {
        lLever.ArmToggle(false);
      }

      /* Cue Commands */
      else if (json["cmd"] == 301) {
        cue.ArmToggle(true);
      } 
      else if (json["cmd"] == 300) {
        cue.ArmToggle(false);
      }

      /* Pump Commands */
      else if (json["cmd"] == 401) {
        pump.ArmToggle(true);
      } 
      else if (json["cmd"] == 400) {
        pump.ArmToggle(false);
      }

      /* Session Commands */
      else if (json["cmd"] == 11) {
        StartSession();
        SetOutputTimestampOffset(SESSION_END_TIMESTAMP);
      }
      else if (json["cmd"] == 10) {
        SESSION_END_TIMESTAMP = millis();
      }

      /* Exception */
      else {
        desc = F("Command does not exist");
        json["level"] = F("error");
        json["desc"] = desc;

        serializeJsonPretty(json, Serial);
      }
    } else {
      desc = F("No valid JSON command exists");
      
      json["level"] = F("error");
      json["desc"] = desc;  
        
      serializeJsonPretty(json, Serial);
    }   
  }
}

void StartSession() {
  JsonDocument json;
  String desc;

  SESSION_START_TIMESTAMP = millis();
  
  desc = F("Session started");

  json["level"] = F("output");
  json["desc"] = desc;
  json["timestamp"] = SESSION_START_TIMESTAMP;

  serializeJsonPretty(json, Serial);
  Serial.println(); 
}

void SetOutputTimestampOffset(uint32_t ts) {
  rLever.SetOffset(ts);
  lLever.SetOffset(ts);
  cue.SetOffset(ts);
  pump.SetOffset(ts);
}
