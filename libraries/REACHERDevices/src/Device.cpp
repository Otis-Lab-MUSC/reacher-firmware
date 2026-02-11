/**
 * @file Device.cpp
 * @brief Device base class implementation.
 */

#include "Device.h"

Device::Device(int8_t pin, uint8_t mode, const char* device) {
  this->pin = pin;
  this->mode = mode;
  this->device = device;
  armed = false;
  offset = 0;
  pinMode(pin, mode);
}

void Device::ArmToggle(bool arm) {
  this->armed = arm;

  Serial.print(F("{\"level\":\"001\",\"device\":\""));
  Serial.print(device);
  Serial.print(F("\",\"pin\":"));
  Serial.print(pin);
  Serial.print(F(",\"desc\":\""));
  Serial.print(this->armed ? F("ARMED") : F("DISARMED"));
  Serial.println(F("\"}"));
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

void logParamChange(const __FlashStringHelper* device,
                    const __FlashStringHelper* param,
                    uint32_t value) {
  Serial.print(F("{\"level\":\"000\",\"device\":\""));
  Serial.print(device);
  Serial.print(F("\",\"param\":\""));
  Serial.print(param);
  Serial.print(F("\",\"value\":"));
  Serial.print(value);
  Serial.println('}');
}

void logParamChange(const __FlashStringHelper* device,
                    const __FlashStringHelper* param,
                    bool value) {
  Serial.print(F("{\"level\":\"000\",\"device\":\""));
  Serial.print(device);
  Serial.print(F("\",\"param\":\""));
  Serial.print(param);
  Serial.print(F("\",\"value\":"));
  Serial.print(value ? F("true") : F("false"));
  Serial.println('}');
}

void logParamChange(const __FlashStringHelper* device,
                    const __FlashStringHelper* param,
                    const __FlashStringHelper* value) {
  Serial.print(F("{\"level\":\"000\",\"device\":\""));
  Serial.print(device);
  Serial.print(F("\",\"param\":\""));
  Serial.print(param);
  Serial.print(F("\",\"value\":\""));
  Serial.print(value);
  Serial.println(F("\"}"));
}
