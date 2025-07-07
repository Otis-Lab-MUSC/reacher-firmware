#include <Arduino.h>
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
  timestamp = 0;
  offset = 0;
  instance = this;
  device = "MICROSCOPE";
  event = "TIMESTAMP";
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
            LogOutput();
            interrupts(); 
        }
    }
}

void Microscope::ArmToggle(bool armed) {
  this->armed = armed;
}

void Microscope::SetOffset(uint32_t offset) {
    this->offset = offset;
}

void Microscope::LogOutput() { 
  JsonDocument doc;

  doc[F("level")] = F("007");
  doc[F("device")] = device;
  doc[F("pin")] = timestampPin;
  doc[F("event")] = event;
  doc[F("timestamp")] = instance->timestamp;

  serializeJson(doc, Serial);
  Serial.println();   
}

byte Microscope::TriggerPin() {
  return triggerPin;
}

byte Microscope::TimestampPin() {
  return timestampPin;
}
