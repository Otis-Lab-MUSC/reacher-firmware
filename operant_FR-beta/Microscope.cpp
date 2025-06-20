#include <Arduino.h>
#include <ArduinoJson.h>

#include "Microscope.h"

Microscope* Microscope::instance = nullptr;

Microscope::Microscope(int8_t triggerPin, int8_t timestampPin) {
  const char deviceType[] = "MICROSCOPE";
  const char eventType[] = "FRAME";
  
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
            LogOutput();
            interrupts(); 
        }
    }
}

void Microscope::ArmToggle(bool armed) {
  doc.clear();
  
  this->armed = armed;
  
  doc["level"] = 444;
  doc["device"] = deviceType;
  doc["pin"] = triggerPin;
  doc["var"] = "armed";
  doc["val"] = this->armed;

  serializeJson(doc, Serial);
  Serial.println();
}

void Microscope::SetOffset(uint32_t offset) {
    this->offset = offset;
}

void Microscope::LogOutput() {
  doc.clear();
  
  doc["level"] = 777;
  doc["device"] = deviceType;
  doc["pin"] = timestampPin;
  doc["event"] = eventType;
  doc["ts"] = instance->timestamp;

  serializeJson(doc, Serial);
  Serial.println();   
}

void Microscope::Config(JsonDocument* doc) {
  JsonObject conf = doc->createNestedObject(deviceType);

  conf["trigger_pin"] = triggerPin;
  conf["timestamp_pin"] = timestampPin;
}

byte Microscope::TriggerPin() {
  return triggerPin;
}

byte Microscope::TimestampPin() {
  return timestampPin;
}
