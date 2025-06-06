#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "SwitchLever.h"

SwitchLever::SwitchLever(int8_t pin, const char* orientation) : Device(pin, INPUT_PULLUP) {
  armed = false;
  this->pin = pin;
  strncpy(this->orientation, orientation, sizeof(this->orientation) - 1);
  this->orientation[sizeof(this->orientation) - 1] = '\0';
  pinMode(pin, INPUT_PULLUP);
  initState = digitalRead(pin);
  previousState = digitalRead(pin);
  stableState = digitalRead(pin);

  reinforced = false;
  debounceDelay = 20;
  timeoutInterval = 0;
}

void SwitchLever::ArmToggle(bool armed) {
  JsonDocument json;
  String desc;
  
  this->armed = armed;
  
  desc = F("Switch lever ");
  desc += armed ? F("armed") : F("disarmed");
  desc += F(" at pin ");
  desc += pin;

  json["level"] = F("info");
  json["desc"] = desc;

  serializeJsonPretty(json, Serial);
  Serial.println();
}

void SwitchLever::Monitor() {
  uint32_t currentTimestamp = millis();
  if (armed) {
    bool currentState = digitalRead(pin);
    if (currentState != previousState) {
      lastDebounceTimestamp = currentTimestamp;
    }
    if ((currentTimestamp - lastDebounceTimestamp) > debounceDelay) {
      if (currentState != stableState) {
        stableState = currentState;
        if (stableState != initState) {
          startTimestamp = currentTimestamp - Offset();
          Classify(startTimestamp);
        } else {
          endTimestamp = currentTimestamp - Offset();
          LogOutput();
        }
      }
    }   
    previousState = currentState;
  }
}

void SwitchLever::SetCue(Cue* cue) {
  this->cue = cue;
}

void SwitchLever::SetPump(Pump* pump) {
  this->pump = pump;
}

void SwitchLever::SetLaser(Laser* laser) {
  this->laser = laser;
}

void SwitchLever::Classify(uint32_t startTimestamp) {
  JsonDocument json;
  String desc;
  
  if (reinforced) {
    if (startTimestamp <= timeoutIntervalEnd) {
      pressType = PressType::TIMEOUT;
    } else {
      pressType = PressType::ACTIVE;
      timeoutIntervalEnd = startTimestamp + timeoutInterval;
      AddActions();
    }
  } else {
    pressType = PressType::INDEPENDENT;
  }  
}

void SwitchLever::SetTimeoutIntervalLength(uint32_t timeoutInterval) {
  JsonDocument json;
  String desc;
  this->timeoutInterval = timeoutInterval;

  desc = F("Timeout interval length set to ");
  desc += this->timeoutInterval;
  desc += F("ms for switch lever at pin ");
  desc += pin;

  json["level"] = F("info");
  json["desc"] = desc;

  serializeJsonPretty(json, Serial);
  Serial.println();
}

void SwitchLever::SetReinforcement(bool reinforced) {
  JsonDocument json;
  String desc;
  this->reinforced = reinforced;

  desc = F("Reinforcement set to ");
  desc += (this->reinforced) ? F("true") : F("false");
  desc += F(" for switch lever at pin ");
  desc += pin;

  json["level"] = F("info");
  json["desc"] = desc;

  serializeJsonPretty(json, Serial);
  Serial.println();
}
 

void SwitchLever::LogOutput() {
  JsonDocument json;
  String desc;

  desc = (reinforced) ? ((pressType == PressType::ACTIVE) ? F("Active") : F("Timeout")) : F("Independent");
  desc += F(" press occurred for ");
  desc += orientation;
  desc += F(" lever");

  json["level"] = F("output");
  json["desc"] = desc;
  json["device"] = F("SWITCH_LEVER");
  json["orientation"] = orientation;
  json["classification"] = (reinforced) ? ((pressType == PressType::ACTIVE) ? F("ACTIVE") : F("TIMEOUT")) : F("INDEPENDENT"); 
  json["start_timestamp"] = startTimestamp;
  json["end_timestamp"] = endTimestamp;
  serializeJsonPretty(json, Serial);
  Serial.println();
}

void SwitchLever::AddActions() {
  if (cue) { cue->SetEvent(); }
  if (pump) { pump->SetEvent(); }
  if (laser) { laser->SetEvent(); } 
}
