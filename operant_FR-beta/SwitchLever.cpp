#include <Arduino.h>
#include <SoftwareSerial.h>

#include "SwitchLever.h"

SwitchLever::SwitchLever(int8_t _pin, int8_t _mode) : Device(_pin, _mode) {
  pin = _pin;
  mode = _mode;
  armed = false;
  previousState = digitalRead(pin);
  stableState = digitalRead(pin);
  pinMode(pin, mode);
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

void SwitchLever::SetOrientation(char _orientation[2]) {
  Serial.print(F("Switch lever orientation set to: "));
  strcpy(orientation, _orientation);
  Serial.println(orientation);
  
}

void SwitchLever::Monitor() {
  static uint32_t lastDebounceTimestamp = 0;
  const uint8_t debounceDelay = 50;
  uint32_t currentTimestamp = millis();
  if (armed) {
    bool currentState = digitalRead(pin);
    if (currentState != previousState) {
      lastDebounceTimestamp = currentTimestamp;
    }
    if ((currentTimestamp - lastDebounceTimestamp) > debounceDelay) {
      stableState = currentState;
      if (stableState == LOW) {
        pressTimestamp = currentTimestamp;
        // FIXME: define press type here
      }
      else if (stableState == HIGH) {
        releaseTimestamp = currentTimestamp;
        // FIXME: send output here
      }
    }
    previousState = currentState;
  }
}

bool SwitchLever::PreviousState() const {
  return previousState;
}

bool SwitchLever::StableState() const {
  return stableState;
}

const char* SwitchLever::Orientation() const {
  return orientation;
}
