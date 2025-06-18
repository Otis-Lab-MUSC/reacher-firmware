#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "Device.h"

#define deviceType "UNKNOWN"
#define eventType "UNKNOWN"

Device::Device(int8_t pin, uint8_t mode) {
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
}

void Device::Config(JsonDocument* json) {
}
