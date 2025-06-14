#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "Device.h"
#include "SwitchLever.h"
#include "Cue.h"
#include "Pump.h"
#include "LickCircuit.h"
#include "Laser.h"
#include "Microscope.h"

#define SKETCH_NAME F("operant_FR.ino")
#define VERSION F("v1.1.1")
#define BAUD_RATE 115200

#define RH_LEVER_PIN 10
#define LH_LEVER_PIN 13
#define CUE_PIN 3
#define PUMP_PIN 4
#define LICK_CIRCUIT_PIN 5
#define LASER_PIN 6
#define MICROSCOPE_TRIGGER_PIN 9
#define MICROSCOPE_TIMESTAMP_PIN 2

#define DEF_CUE_FREQ 8000
#define DEF_CUE_DUR 1600
#define DEF_CUE_TRACE 0

#define DEF_PUMP_DUR 2000
#define DEF_PUMP_TRACE DEF_CUE_DUR

#define DEF_LASER_FREQ 1
#define DEF_LASER_DUR 5000
#define DEF_LASER_TRACE 0

SwitchLever rLever(RH_LEVER_PIN, "RH");
SwitchLever lLever(LH_LEVER_PIN, "LH");
Cue cue(CUE_PIN, DEF_CUE_FREQ, DEF_CUE_DUR, DEF_CUE_TRACE);
Pump pump(PUMP_PIN, DEF_PUMP_DUR, cue.Duration());
LickCircuit lickCircuit(LICK_CIRCUIT_PIN);
Laser laser(LASER_PIN, 40, DEF_LASER_DUR, cue.Duration());
Microscope microscope(MICROSCOPE_TRIGGER_PIN, MICROSCOPE_TIMESTAMP_PIN);

uint32_t SESSION_START_TIMESTAMP;
uint32_t SESSION_END_TIMESTAMP;

void setup() {
  delay(100);
  Serial.begin(BAUD_RATE);
  delay(100);

  JsonDocument json;

  json["level"] = F("SETUP");
  json["sketch"] = SKETCH_NAME;
  json["version"] = VERSION;
  json["baud_rate"] = BAUD_RATE;

  // FIXME: add device pins here

  serializeJson(json, Serial);
  Serial.println();
  
  cue.Jingle();

  rLever.SetCue(&cue);
  rLever.SetPump(&pump);
  rLever.SetLaser(&laser);
  rLever.SetTimeoutIntervalLength(DEF_CUE_DUR + DEF_PUMP_DUR);
  rLever.SetReinforcement(true);
  
//  rLever.ArmToggle(true);
//  lLever.ArmToggle(true);
//  cue.ArmToggle(true);
//  pump.ArmToggle(true);
//  lickCircuit.ArmToggle(true);
//  laser.ArmToggle(true);
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
    JsonDocument json;
    String cmd = Serial.readStringUntil('\n');
    String desc;
    DeserializationError error = deserializeJson(json, cmd);

    if (error) {
      desc = F("Command parsing failed: ");
      desc += error.f_str();
      
      json["level"] = F("PROGERR");
      json["desc"] = desc;

      serializeJson(json, Serial);
      Serial.println();
    } 
    
    else if(json["cmd"]) {

      /* RH Lever Commands */
      if (json["cmd"] == 1001) {
        rLever.ArmToggle(true);
      } 
      else if (json["cmd"] == 1000) {
        rLever.ArmToggle(false);
      }
      else if (json["cmd"] == 1074) {
        rLever.SetTimeoutIntervalLength(json["timeout"]);
      }
      else if (json["cmd"] == 1081) {
        rLever.SetReinforcement(true);
      }
      else if (json["cmd"] == 1080) {
        rLever.SetReinforcement(false);
      }

      /* LH Lever Commands */
      else if (json["cmd"] == 1301) {
        lLever.ArmToggle(true);
      } 
      else if (json["cmd"] == 1300) {
        lLever.ArmToggle(false);
      }
      else if (json["cmd"] == 1374) {
        lLever.SetTimeoutIntervalLength(json["timeout"]);
      }
      else if (json["cmd"] == 1381) {
        lLever.SetReinforcement(true);
      }
      else if (json["cmd"] == 1380) {
        lLever.SetReinforcement(false);
      }

      /* Cue Commands */
      else if (json["cmd"] == 301) {
        cue.ArmToggle(true);
      } 
      else if (json["cmd"] == 300) {
        cue.ArmToggle(false);
      }
      else if (json["cmd"] == 371) {
        cue.SetFrequency(json["frequency"]);
      }
      else if (json["cmd"] == 372) {
        cue.SetDuration(json["duration"]);
      }
      else if (json["cmd"] == 373) {
        cue.SetTraceInterval(json["trace"]);
      }

      /* Pump Commands */
      else if (json["cmd"] == 401) {
        pump.ArmToggle(true);
      } 
      else if (json["cmd"] == 400) {
        pump.ArmToggle(false);
      }
      else if (json["cmd"] == 472) {
        pump.SetDuration(json["duration"]);
      }
      else if (json["cmd"] == 473) {
        pump.SetTraceInterval(json["trace"]);
      }

      /* Lick Circuit Commands */
      else if (json["cmd"] == 501) {
        lickCircuit.ArmToggle(true);
      } 
      else if (json["cmd"] == 500) {
        lickCircuit.ArmToggle(false);
      }

      /* Laser Commands */
      else if (json["cmd"] == 601) {
        laser.ArmToggle(true);
      } 
      else if (json["cmd"] == 600) {
        laser.ArmToggle(false);
      }
      else if(json["cmd"] == 671) {
        laser.SetFrequency(json["frequency"]);
      }
      else if(json["cmd"] == 672) {
        laser.SetDuration(json["duration"]);
      }
      else if(json["cmd"] == 673) {
        laser.SetTraceInterval(json["trace"]);
      }
      else if (json["cmd"] == 681) {
        laser.SetMode(true);
      }
      else if (json["cmd"] == 682) {
        laser.SetMode(false);
      }

      /* Session Commands */
      else if (json["cmd"] == 101) {
        StartSession();
        SetDeviceTimestampOffset(SESSION_START_TIMESTAMP);
      }
      else if (json["cmd"] == 100) {
        EndSession();
        ArmToggleDevices(false);
      }

      /* Exception */
      else {
        desc = F("Command does not exist");
        
        json["level"] = F("PROGERR");
        json["desc"] = desc;

        serializeJson(json, Serial);
        Serial.println();
      }
    } else {
      desc = F("No valid JSON command exists");
      
      json["level"] = F("PROGERR");
      json["desc"] = desc;  
        
      serializeJson(json, Serial);
      Serial.println();
    }   
  }
}

void StartSession() {
  JsonDocument json;
  String desc;

  SESSION_START_TIMESTAMP = millis();
  
  desc = F("Session started");

  json["level"] = F("PROGOUT");
  json["desc"] = desc;
  json["device"] = F("CONTROLLER");
  json["classification"] = F("START");
  json["start_timestamp"] = 0;
  json["end_timestamp"] = 0;

  serializeJson(json, Serial);
  Serial.println(); 
}

void EndSession() {
  JsonDocument json;
  String desc;

  SESSION_END_TIMESTAMP = millis();  
  desc = F("Session ended");

  json["level"] = F("PROGOUT");
  json["desc"] = desc;
  json["device"] = F("CONTROLLER");
  json["classification"] = F("END");
  json["start_timestamp"] = SESSION_END_TIMESTAMP - SESSION_START_TIMESTAMP;
  json["end_timestamp"] = SESSION_END_TIMESTAMP - SESSION_START_TIMESTAMP;

  // manually write LOW signals before shut off
  noTone(CUE_PIN);
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(LASER_PIN, LOW);

  serializeJson(json, Serial);
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
}
