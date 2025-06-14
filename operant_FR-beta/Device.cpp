#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "Device.h"

Device::Device(int8_t pin, uint8_t mode) {
  this->pin = pin;
  this->mode = mode;
  armed = false;
  pinMode(pin, mode);
  offset = 0;
}

void Device::ArmToggle(bool arm) {
  JsonDocument json;
  String desc;
  
  this->armed = armed;
  
  desc = F("Device ");
  desc += armed ? F("armed") : F("disarmed");
  desc += F(" at pin ");
  desc += pin;

  json["level"] = F("PROGINFO");
  json["desc"] = desc;

  serializeJson(json, Serial);
  Serial.println();
}

void Device::SetOffset(uint32_t offset) {
  this->offset = offset;
}

int8_t Device::Pin() const {
  return pin;
}

bool Device::Armed() const {
  return armed;
}

uint32_t Device::Offset() const {
  return offset;
}

void Device::LogOutput() {
  JsonDocument json;
  String desc;
    
  desc = F("Event occurred for device at pin ");
  desc += pin;

  json["level"] = F("PROGINFO");
  json["desc"] = desc;
  json["offset"] = Offset();

  serializeJson(json, Serial);
  Serial.println();  
}
