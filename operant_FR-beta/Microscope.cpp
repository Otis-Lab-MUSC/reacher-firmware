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
            LogOutput();
            interrupts(); 
        }
    }
}

void Microscope::ArmToggle(bool armed) {
  JsonDocument json;
  this->armed = armed;
  
  json["level"] = F("PROGINFO");
  json["device"] = F("MICROSCOPE");
  json["pin"] = triggerPin;
  json["desc"] = armed ? F("Microscope armed") : F("Microscope disarmed");

  serializeJson(json, Serial);
  Serial.println();
}

void Microscope::SetOffset(uint32_t offset) {
    this->offset = offset;
}

void Microscope::LogOutput() {
  JsonDocument json;

  json["level"] = F("PROGOUT");
  json["device"] = F("MICROSCOPE");
  json["pin"] = timestampPin;
  json["event"] = F("FRAME");
  json["timestamp"] = instance->timestamp;
  json["desc"] = F("Frame timestamp collected");

  serializeJson(json, Serial);
  Serial.println();   
}
