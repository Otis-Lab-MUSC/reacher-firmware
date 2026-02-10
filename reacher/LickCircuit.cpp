#include <ArduinoJson.h>
#include "LickCircuit.h"

LickCircuit::LickCircuit(int8_t pin)
  : Device(pin, INPUT_PULLUP, "LICK_CIRCUIT") {
  initState = digitalRead(pin);
  previousState = digitalRead(pin);
  stableState = digitalRead(pin);
  lastDebounceTimestamp = 0;
  debounceDelay = 20;
  startTimestamp = 0;
  endTimestamp = 0;
  callback = nullptr;
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
          if (callback) {
            callback(DeviceType::LICK, currentTimestamp);
          }
        } else {
          endTimestamp = currentTimestamp;
          LogOutput();
        }
      }
    }
    previousState = currentState;
  }
}

void LickCircuit::SetCallback(InputEventCallback cb) {
  callback = cb;
}

void LickCircuit::LogOutput() {
  JsonDocument doc;

  doc[F("level")] = F("007");
  doc[F("device")] = device;
  doc[F("pin")] = pin;
  doc[F("event")] = F("LICK");
  doc[F("start_timestamp")] = startTimestamp - Offset();
  doc[F("end_timestamp")] = endTimestamp - Offset();

  serializeJson(doc, Serial);
  Serial.println();
}
