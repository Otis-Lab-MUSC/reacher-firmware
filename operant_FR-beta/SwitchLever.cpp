#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "SwitchLever.h"

SwitchLever::SwitchLever(int8_t _pin, int8_t _mode, const char* _orientation) : Device(_pin, _mode) {
  armed = false;
  pin = _pin;
  mode = _mode;
  pinMode(pin, mode);
  previousState = HIGH;
  stableState = HIGH;
  strncpy(orientation, _orientation, sizeof(orientation) - 1);
  orientation[sizeof(orientation) - 1] = '\0';
  debounceDelay = 50;
  timeoutInterval = 20000;
}

void SwitchLever::ArmToggle() {
  armed = !armed;
  Serial.print(F("Switch lever "));
  Serial.print(armed ? F("armed") : F("disarmed"));
  Serial.print(F(" at pin: "));
  Serial.println(pin);
}

void SwitchLever::SetPreviousState(bool _previousState) {
  previousState = _previousState;  
}

void SwitchLever::SetStableState(bool _stableState) {
  stableState = _stableState; 
}

void SwitchLever::Monitor() {
  uint32_t currentTimestamp = millis();
  JsonDocument json;
  if (armed) {
    bool currentState = digitalRead(pin);
    if (currentState != previousState) {
      lastDebounceTimestamp = currentTimestamp;
    }
    if ((currentTimestamp - lastDebounceTimestamp) > debounceDelay) {
      if (currentState != stableState) {
        stableState = currentState;
        if (stableState == LOW) {
          pressTimestamp = currentTimestamp;
//          if (pressTimestamp <= timeoutIntervalEnd) {
//            pressType = PressType::TIMEOUT;
//          } else {
//            pressType = PressType::ACTIVE;
//            timeoutIntervalEnd = pressTimestamp + timeoutInterval;
//          }
        } else {
          releaseTimestamp = currentTimestamp;
          json["device"] = F("SWITCH_LEVER");
          json["orientation"] = orientation;
//          json["classification"] = (pressType == PressType::ACTIVE) ? "ACTIVE" : "TIMEOUT";
          json["press_timestamp"] = pressTimestamp;
          json["release_timestamp"] = releaseTimestamp;
          serializeJsonPretty(json, Serial);
          Serial.println();
        }
      }
    }   
    previousState = currentState;
  }
}

SwitchLever::PressType SwitchLever::ClassifyPress(uint32_t _pressTimestamp) {
  
}
