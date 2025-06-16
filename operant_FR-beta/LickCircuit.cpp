#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "LickCircuit.h"

LickCircuit::LickCircuit(int8_t pin) : Device(pin, INPUT_PULLUP) {
  armed = false;
  this->pin = pin;
  pinMode(pin, INPUT_PULLUP);
  initState = digitalRead(pin);
  previousState = digitalRead(pin);
  stableState = digitalRead(pin);
  debounceDelay = 20;
}

void LickCircuit::ArmToggle(bool armed) {
  JsonDocument json;
  this->armed = armed;
  
  json["level"] = F("PROGINFO");
  json["device"] = F("LICK_CIRCUIT");
  json["pin"] = pin;
  json["desc"] = armed ? F("Lick circuit armed") : F("Lick circuit disarmed");

  serializeJson(json, Serial);
  Serial.println();
}

void LickCircuit::Monitor(uint32_t currentTimestamp) {
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
        } else {
          endTimestamp = currentTimestamp;
          LogOutput();
        }
      }
    }   
    previousState = currentState;
  }
}

void LickCircuit::LogOutput() {
  JsonDocument json;

  json["level"] = F("PROGOUT");
  json["device"] = F("LICK_CIRCUIT");
  json["pin"] = pin;
  json["event"] = F("LICK");
  json["start_timestamp"] = startTimestamp - Offset();
  json["end_timestamp"] = endTimestamp - Offset();
  json["desc"] = F("Lick occurred");
  
  serializeJson(json, Serial);
  Serial.println();
}
