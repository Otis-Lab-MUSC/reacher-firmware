#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "Microscope.h"

Microscope* Microscope::instance = nullptr;

Microscope::Microscope(int8_t triggerPin, int8_t timestampPin) {
  this->triggerPin = triggerPin;
  this->timestampPin = timestampPin;
  pinMode(this->triggerPin, OUTPUT);
  pinMode(this->timestampPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(this->timestampPin), TimestampISR, RISING);
  received = false;
  armed = false;
  timestamp = 0;
  offset = 0;
  instance = this;
}

static void Microscope::TimestampISR() {
  if (instance) {
    instance->received = true;
    instance->timestamp = millis() - instance->offset;
  }
}

void Microscope::HandleFrameSignal() {
    if (armed) {
        if (received) {
            noInterrupts();
            received = false;
            int32_t t = timestamp;
            interrupts(); 
            Serial.println("FRAME_TIMESTAMP," + String(t));
        }
    }
}

void Microscope::ArmToggle(bool armed) {
    this->armed = armed;
}

void Microscope::SetOffset(uint32_t offset) {
    this->offset = offset;
}
