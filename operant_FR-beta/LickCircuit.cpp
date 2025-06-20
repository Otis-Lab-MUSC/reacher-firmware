#include <Arduino.h>
#include <ArduinoJson.h>

#include "LickCircuit.h"

LickCircuit::LickCircuit(int8_t pin) : Device(pin, INPUT_PULLUP) {
  const char deviceType[] = "LICK_CIRCUIT";
  const char eventType[] = "LICK";

  armed = false;
  this->pin = pin;
  pinMode(pin, INPUT_PULLUP);
  initState = digitalRead(pin);
  previousState = digitalRead(pin);
  stableState = digitalRead(pin);
  debounceDelay = 20;
}

void LickCircuit::ArmToggle(bool armed) {
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
  doc.clear();
  
  doc["level"] = 777;
  doc["device"] = deviceType;
  doc["pin"] = pin;
  doc["event"] = eventType;
  doc["ts1"] = startTimestamp - Offset();
  doc["ts2"] = endTimestamp - Offset();
  
  serializeJson(doc, Serial);
  Serial.println();
}

void LickCircuit::Config(JsonDocument* doc) {
  JsonObject conf = doc->createNestedObject(deviceType);

  conf["pin"] = pin;
  conf["debounce"] = debounceDelay;
}
