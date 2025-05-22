#include <Arduino.h>
#include <SoftwareSerial.h>

#include "SwitchLever.h"

SwitchLever::SwitchLever(byte pin, int8_t mode) : Device(pin, mode) {
  armed = false;
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

bool SwitchLever::PreviousState() const {
  return previousState;
}

bool SwitchLever::StableState() const {
  return stableState;
}

char SwitchLever::Orientation() const {
  return orientation;
}
