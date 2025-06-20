#include <Arduino.h>
#include <ArduinoJson.h>

#include "Device.h"

Device::Device(int8_t pin, uint8_t mode) {
  const char deviceType[] = "UNKNOWN";
  const char eventType[] = "UNKNOWN";

  this->pin = pin;
  this->mode = mode;
  armed = false;
  pinMode(pin, mode);
  offset = 0;
}

void Device::ArmToggle(bool arm) {
  this->armed = armed;
}

void Device::SetOffset(uint32_t offset) {
  this->offset = offset;
}

byte Device::Pin() const {
  return pin;
}

bool Device::Armed() const {
  return armed;
}

uint32_t Device::Offset() const {
  return offset;
}

void Device::LogOutput() {
}

void Device::Config(JsonDocument* doc) {
}

void Device::SerializeVar(const char var[], bool val) {
  doc.clear();
  doc["level"] = 444;
  doc["device"] = String(deviceType);
  doc["pin"] = pin;
  doc["var"] = String(var);
  doc["val"] = val;
  serializeJson(doc, Serial);
  Serial.println();
}

void Device::SerializeVar(const char var[], uint32_t val) {
  doc.clear();
  doc["level"] = 444;
  doc["device"] = String(deviceType);
  doc["pin"] = pin;
  doc["var"] = String(var);
  doc["val"] = val;
  serializeJson(doc, Serial);
  Serial.println();
}
