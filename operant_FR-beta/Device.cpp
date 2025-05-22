#include <Arduino.h>
#include <SoftwareSerial.h>

#include "Device.h"

Device::Device(byte pin, uint8_t mode) {
  armed = false;
  pinMode(pin, mode);
}

void Device::ArmToggle() {
  armed = !armed;
  Serial.print(F("Device "));
  Serial.print(armed ? F("armed") : F("disarmed"));
  Serial.print(F(" at pin: "));
  Serial.println(pin);
}

byte Device::Pin() const {
  return pin;
}

bool Device::Armed() const {
  return armed;
}
