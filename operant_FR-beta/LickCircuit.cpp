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
  String desc;
  
  this->armed = armed;
  
  desc = F("Lick circuit ");
  desc += armed ? F("armed") : F("disarmed");
  desc += F(" at pin ");
  desc += pin;

  json["level"] = F("info");
  json["desc"] = desc;
  json["device"] = F("LICK_CIRCUIT");
  json["start_timestamp"] = startTimestamp;
  json["end_timestamp"] = endTimestamp;

  serializeJsonPretty(json, Serial);
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
  String desc;

  desc += F("Lick occurred for circuit at pin ");
  desc += pin;

  json["level"] = F("output");
  json["desc"] = desc;
  json["device"] = F("LICK_CIRCUIT");
  json["onset_timestamp"] = startTimestamp - Offset();
  json["Offset()_timestamp"] = endTimestamp - Offset();
  serializeJsonPretty(json, Serial);
  Serial.println();
}
