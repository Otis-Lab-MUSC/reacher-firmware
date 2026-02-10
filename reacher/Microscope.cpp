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

void Microscope::TimestampISR() {
  if (instance) {
    instance->received = true;
    instance->timestamp = millis() - instance->offset;
  }
}

void Microscope::HandleFrameSignal() {
  if (armed && received) {
    noInterrupts();
    received = false;
    interrupts();
    LogOutput();
  }
}

void Microscope::ArmToggle(bool armed) {
  this->armed = armed;
}

void Microscope::SetOffset(uint32_t offset) {
  this->offset = offset;
}

void Microscope::Trigger() {
  digitalWrite(triggerPin, HIGH);
  delay(50);
  digitalWrite(triggerPin, LOW);
}

byte Microscope::TriggerPin() const {
  return triggerPin;
}

byte Microscope::TimestampPin() const {
  return timestampPin;
}

void Microscope::LogOutput() {
  JsonDocument doc;

  doc[F("level")] = F("008");
  doc[F("device")] = F("MICROSCOPE");
  doc[F("pin")] = timestampPin;
  doc[F("event")] = F("TIMESTAMP");
  doc[F("timestamp")] = timestamp;

  serializeJson(doc, Serial);
  Serial.println();
}
