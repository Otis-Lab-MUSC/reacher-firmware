#include <Arduino.h>
#include <ArduinoJson.h>

#include "SwitchLever.h"

SwitchLever::SwitchLever(int8_t pin, const char* orientation) : Device(pin, INPUT_PULLUP, "SWITCH_LEVER", "PRESS") {  
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
  ratio = 1; // default schedule FR1
  numPresses = 0;
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

void SwitchLever::SetRatio(uint8_t ratio) {
  this->ratio = ratio;
}

void SwitchLever::Classify(uint32_t startTimestamp, uint32_t currentTimestamp) {
  if (reinforced) {
    if (startTimestamp <= timeoutIntervalEnd) {
      pressType = PressType::TIMEOUT;
    } else {
      pressType = PressType::ACTIVE;
      timeoutIntervalEnd = startTimestamp + timeoutInterval;
      numPresses++;
      if (numPresses == ratio) {
        AddActions(currentTimestamp);
        numPresses = 0;
      }
    }
  } else {
    pressType = PressType::INACTIVE;
  }  
}

void SwitchLever::SetTimeoutIntervalLength(uint32_t timeoutInterval) {
  this->timeoutInterval = timeoutInterval;
}

void SwitchLever::SetActiveLever(bool reinforced) { 
  this->reinforced = reinforced;
}
 
void SwitchLever::LogOutput() {
  JsonDocument doc;
  
  doc[F("level")] = F("007");
  doc[F("device")] = device;
  doc[F("pin")] = pin;
  doc[F("event")] = event;
  doc[F("class")] = (pressType == 0) ? F("INACTIVE") : ((pressType == 1) ? F("ACTIVE") : F("TIMEOUT"));
  doc[F("start_timestamp")] = startTimestamp - Offset();
  doc[F("end_timestamp")] = endTimestamp - Offset();
  doc[F("orientation")] = orientation;
  
  serializeJson(doc, Serial);
  Serial.println();
}

void SwitchLever::AddActions(uint32_t currentTimestamp) {
  if (cue) { cue->SetEvent(currentTimestamp); }
  if (pump) { pump->SetEvent(currentTimestamp); }
  if (laser) { laser->SetEvent(currentTimestamp); }
}

JsonDocument SwitchLever::Defaults() {
  JsonDocument defaults;

  defaults[F("level")] = F("000"); 
  defaults[F("device")] = device;
  defaults[F("pin")] = pin;
  defaults[F("orientation")] = orientation;
  defaults[F("reinforced")] = reinforced ? F("REINFORCED") : F("UNENFORCED");
  defaults[F("timeout")] = timeoutInterval;
  defaults[F("ratio")] = ratio;

  return defaults;
}
