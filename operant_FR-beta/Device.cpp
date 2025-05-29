#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "Device.h"

Device::Device(int8_t pin, uint8_t mode) {
  this->pin = pin;
  this->mode = mode;
  armed = false;
  pinMode(pin, mode);
}

void Device::ArmToggle(bool arm) {
  JsonDocument json;
  String desc;
  
  this->armed = armed;
  
  desc = F("Device ");
  desc += armed ? F("armed") : F("disarmed");
  desc += F(" at pin ");
  desc += pin;

  json["level"] = F("info");
  json["desc"] = desc;

  serializeJsonPretty(json, Serial);
  Serial.println();
}

void Device::EventHandler() {
  JsonDocument json;
  String desc;
  
  armed = !armed;
  
  desc = F("Event occured at pin ");
  desc += pin;

  json["level"] = F("info");
  json["desc"] = desc;

  serializeJsonPretty(json, Serial);
  Serial.println();
}

int8_t Device::Pin() const {
  return pin;
}

bool Device::Armed() const {
  return armed;
}
