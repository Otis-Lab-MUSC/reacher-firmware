#include <Arduino.h>
#include <SoftwareSerial.h>

#include "Device.h"

Device::Device(int8_t _pin, uint8_t _mode) {
  pin = _pin;
  mode = _mode;
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

int8_t Device::Pin() const {
  return pin;
}

bool Device::Armed() const {
  return armed;
}
