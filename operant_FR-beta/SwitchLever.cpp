#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "SwitchLever.h"

#define deviceType "SWITCH_LEVER"
#define eventType "PRESS"

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
  this->armed = armed;
  
  json["level"] = 444;
  json["device"] = deviceType;
  json["pin"] = pin;
  json["var"] = "armed";
  json["val"] = this->armed;

  serializeJson(json, Serial);
  Serial.println();
}

void SwitchLever::Monitor(uint32_t currentTimestamp) {
  if (armed) {
    bool currentState = digitalRead(pin);
    if (currentState != previousState) {
      lastDebounceTimestamp = currentTimestamp;
    }
    if ((currentTimestamp - lastDebounceTimestamp) > debounceDelay) {
      if (currentState != stableState) {
        stableState = currentState;
        if (stableState != initState) {
          startTimestamp = currentTimestamp;
          Classify(startTimestamp, currentTimestamp);
        } else {
          endTimestamp = currentTimestamp;
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

void SwitchLever::Classify(uint32_t startTimestamp, uint32_t currentTimestamp) {
  if (reinforced) {
    if (startTimestamp <= timeoutIntervalEnd) {
      pressType = PressType::TIMEOUT;
    } else {
      pressType = PressType::ACTIVE;
      timeoutIntervalEnd = startTimestamp + timeoutInterval;
      AddActions(currentTimestamp);
    }
  } else {
    pressType = PressType::INDEPENDENT;
  }  
}

void SwitchLever::SetTimeoutIntervalLength(uint32_t timeoutInterval) {
  JsonDocument json;
  this->timeoutInterval = timeoutInterval;

  json["level"] = 444;
  json["device"] = deviceType;
  json["pin"] = pin;
  json["var"] = "timeoutInterval";
  json["val"] = this->timeoutInterval;

  serializeJson(json, Serial);
  Serial.println();
}

void SwitchLever::SetReinforcement(bool reinforced) {
  JsonDocument json;
  this->reinforced = reinforced;

  json["level"] = 444;
  json["device"] = deviceType;
  json["pin"] = pin;
  json["var"] = "reinforced";
  json["val"] = this->reinforced;

  serializeJson(json, Serial);
  Serial.println();
}
 
void SwitchLever::LogOutput() {
  JsonDocument json;

  json["level"] = 777;
  json["device"] = deviceType;
  json["pin"] = pin;
  json["event"] = eventType;
  json["ts1"] = startTimestamp - Offset();
  json["ts2"] = endTimestamp - Offset();
  json["orient"] = orientation;
  json["class"] = pressType; // 0=ACTIVE, 1=TIMEOUT, 2=INDEPENDENT 
  
  serializeJson(json, Serial);
  Serial.println();
}

void SwitchLever::AddActions(uint32_t currentTimestamp) {
  if (cue) { cue->SetEvent(currentTimestamp); }
  if (pump) { pump->SetEvent(currentTimestamp); }
  if (laser) { laser->SetEvent(currentTimestamp); }
}

void SwitchLever::Config(JsonDocument* json) {
  String device = orientation;
  device += "_LEVER";
  JsonObject conf = json->createNestedObject(device);

  conf["pin"] = pin;
  conf["reinforced"] = reinforced;
  conf["debounce"] = debounceDelay;
  conf["timeout"] = timeoutInterval;
}
