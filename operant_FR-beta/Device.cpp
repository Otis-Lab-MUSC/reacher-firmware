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
  this->armed = armed;
  
  json["level"] = F("PROGINFO");
  json["device"] = F("DEVICE");
  json["pin"] = pin;
  json["desc"] = armed ? F("Device armed") : F("Device disarmed");

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

  json["level"] = F("PROGINFO");
  json["device"] = F("DEVICE");
  json["pin"] = pin;
  json["event"] = F("UNKNOWN");
  json["desc"] = F("Event occurred");

  serializeJson(json, Serial);
  Serial.println();  
}
