#include <Arduino.h>
#include <ArduinoJson.h>

#include "SwitchLever.h"

SwitchLever::SwitchLever(int8_t pin, const char* orientation) : Device(pin, INPUT_PULLUP) {
  const char deviceType[] = "SWITCH_LEVER";
  const char eventType[] = "PRESS";
  
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
  doc.clear();
  
  this->armed = armed;
  
  doc["level"] = 444;
  doc["device"] = deviceType;
  doc["pin"] = pin;
  doc["var"] = "armed";
  doc["val"] = this->armed;

  serializeJson(doc, Serial);
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
  doc.clear();

  this->timeoutInterval = timeoutInterval;

  doc["level"] = 444;
  doc["device"] = deviceType;
  doc["pin"] = pin;
  doc["var"] = "timeoutInterval";
  doc["val"] = this->timeoutInterval;

  serializeJson(doc, Serial);
  Serial.println();
}

void SwitchLever::SetReinforcement(bool reinforced) {
  doc.clear();
  
  this->reinforced = reinforced;

  doc["level"] = 444;
  doc["device"] = deviceType;
  doc["pin"] = pin;
  doc["var"] = "reinforced";
  doc["val"] = this->reinforced;

  serializeJson(doc, Serial);
  Serial.println();
}
 
void SwitchLever::LogOutput() {
  doc.clear();
  
  doc["level"] = 777;
  doc["device"] = deviceType;
  doc["pin"] = pin;
  doc["event"] = eventType;
  doc["ts1"] = startTimestamp - Offset();
  doc["ts2"] = endTimestamp - Offset();
  doc["orient"] = orientation;
  doc["class"] = pressType; // 0=ACTIVE, 1=TIMEOUT, 2=INDEPENDENT 
  
  serializeJson(doc, Serial);
  Serial.println();
}

void SwitchLever::AddActions(uint32_t currentTimestamp) {
  if (cue) { cue->SetEvent(currentTimestamp); }
  if (pump) { pump->SetEvent(currentTimestamp); }
  if (laser) { laser->SetEvent(currentTimestamp); }
}

void SwitchLever::Config(JsonDocument* doc) {
  String device = orientation;
  device += "_LEVER";
  JsonObject conf = doc->createNestedObject(device);

  conf["pin"] = pin;
  conf["reinforced"] = reinforced;
  conf["debounce"] = debounceDelay;
  conf["timeout"] = timeoutInterval;
}
