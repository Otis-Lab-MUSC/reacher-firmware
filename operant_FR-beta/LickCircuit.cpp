#include <Arduino.h>
#include <ArduinoJson.h>

#include "LickCircuit.h"

LickCircuit::LickCircuit(int8_t pin) : Device(pin, INPUT_PULLUP, "LICK_CIRCUIT", "LICK") {
  this->pin = pin;
  pinMode(pin, INPUT_PULLUP);
  initState = digitalRead(pin);
  previousState = digitalRead(pin);
  stableState = digitalRead(pin);
  debounceDelay = 20;
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
  JsonDocument doc;
  
  doc[F("level")] = F("007");
  doc[F("device")] = device;
  doc[F("pin")] = pin;
  doc[F("event")] = event;
  doc[F("start_timestamp")] = startTimestamp - Offset();
  doc[F("end_timestamp")] = endTimestamp - Offset();
  
  serializeJson(doc, Serial);
  Serial.println();
}

JsonDocument LickCircuit::Settings() {
  JsonDocument Settings;

  Settings[F("level")] = F("000"); 
  Settings[F("device")] = device;
  Settings[F("pin")] = pin;

  return Settings;
}
