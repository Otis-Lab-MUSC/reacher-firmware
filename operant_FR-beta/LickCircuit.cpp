#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "LickCircuit.h"

#define deviceType "LICK_CIRCUIT"
#define eventType "LICK"

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
  
  json["level"] = 444;
  json["device"] = deviceType;
  json["pin"] = pin;
  json["var"] = "armed";
  json["val"] = this->armed;

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

  json["level"] = 777;
  json["device"] = deviceType;
  json["pin"] = pin;
  json["event"] = eventType;
  json["ts1"] = startTimestamp - Offset();
  json["ts2"] = endTimestamp - Offset();
  
  serializeJson(json, Serial);
  Serial.println();
}

void LickCircuit::Config(JsonDocument* json) {
  JsonObject conf = json->createNestedObject(deviceType);

  conf["pin"] = pin;
  conf["debounce"] = debounceDelay;
}
