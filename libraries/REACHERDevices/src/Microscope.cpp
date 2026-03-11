/**
 * @file Microscope.cpp
 * @brief Microscope implementation — ISR handling, trigger pulses, and frame logging.
 */

#include "Microscope.h"

Microscope* Microscope::instance = nullptr;

Microscope::Microscope(int8_t triggerPin, int8_t timestampPin) {
  this->triggerPin = triggerPin;
  this->timestampPin = timestampPin;
  pinMode(this->triggerPin, OUTPUT);
  pinMode(this->timestampPin, INPUT_PULLUP); // Prevent floating when disconnected
  attachInterrupt(digitalPinToInterrupt(this->timestampPin), TimestampISR, RISING);
  received = false;
  armed = false;
  timestamp = 0;
  offset = 0;
  triggerActive = false;
  triggerStart = 0;
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
    uint32_t ts = timestamp; // atomic copy of volatile 32-bit value
    interrupts();
    LogOutput(ts);
  }
}

void Microscope::ArmToggle(bool armed) {
  this->armed = armed;
  Serial.print(F("{\"level\":\"001\",\"device\":\"MICROSCOPE\",\"pin\":"));
  Serial.print(triggerPin);
  Serial.print(F(",\"desc\":\""));
  Serial.print(armed ? F("ARMED") : F("DISARMED"));
  Serial.println(F("\"}"));
}

void Microscope::SetOffset(uint32_t offset) {
  this->offset = offset;
}

// Fix: FW-001 — Non-blocking trigger; Trigger() starts the pulse, TickTrigger() ends it.
void Microscope::Trigger() {
  digitalWrite(triggerPin, HIGH);
  triggerActive = true;
  triggerStart = millis();
}

void Microscope::TickTrigger(uint32_t now) {
  if (triggerActive && (now - triggerStart) >= TRIGGER_DURATION_MS) {
    digitalWrite(triggerPin, LOW);
    triggerActive = false;
  }
}

bool Microscope::Armed() const { return armed; }

byte Microscope::TriggerPin() const {
  return triggerPin;
}

byte Microscope::TimestampPin() const {
  return timestampPin;
}

void Microscope::LogOutput(uint32_t ts) {
  Serial.print(F("{\"level\":\"008\",\"device\":\"MICROSCOPE\",\"pin\":"));
  Serial.print(timestampPin);
  Serial.print(F(",\"event\":\"TIMESTAMP\",\"timestamp\":"));
  Serial.print(ts);
  Serial.println('}');
}
