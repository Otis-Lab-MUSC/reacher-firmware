#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "SwitchLever.h"

SwitchLever::SwitchLever(int8_t _pin, const char* _orientation, bool _reinforced) : Device(_pin, INPUT_PULLUP) {
  armed = false;
  pin = _pin;
  pinMode(pin, INPUT_PULLUP);
  initState = digitalRead(pin);
  previousState = digitalRead(pin);
  stableState = digitalRead(pin);
  strncpy(orientation, _orientation, sizeof(orientation) - 1);
  orientation[sizeof(orientation) - 1] = '\0';
  reinforced = _reinforced;
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
        if (stableState != initState) {
          pressTimestamp = currentTimestamp;
          if (reinforced) {
            if (pressTimestamp <= timeoutIntervalEnd) {
              pressType = PressType::TIMEOUT;
            } else {
              pressType = PressType::ACTIVE;
              timeoutIntervalEnd = pressTimestamp + timeoutInterval;
              // FIXME: add event handler for reward delivery
            }
          } else {
            pressType = PressType::INDEPENDENT;
          }
        } else {
          releaseTimestamp = currentTimestamp;
          json["device"] = F("SWITCH_LEVER");
          json["orientation"] = orientation;
          json["classification"] = (reinforced) ? ((pressType == PressType::ACTIVE) ? F("ACTIVE") : F("TIMEOUT")) : F("INDEPENDENT"); 
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
