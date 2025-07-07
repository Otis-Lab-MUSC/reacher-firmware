#include <Arduino.h>
#include <ArduinoJson.h>

#include "Device.h"

Device::Device(int8_t pin, uint8_t mode, const char* device, const char* event) {
  this->pin = pin;
  this->mode = mode;
  this->device = device; 
  this->event = event; 
  armed = false;
  pinMode(pin, mode);
  offset = 0;
}

void Device::ArmToggle(bool arm) { 
  JsonDocument doc;
  
  this->armed = arm; 

  doc[F("level")] = F("001");
  doc[F("device")] = device;
  doc[F("pin")] = pin;
  doc[F("event")] = this->armed ? F("ARMED") : F("DISARMED");

  serializeJson(doc, Serial);
  Serial.println();
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
