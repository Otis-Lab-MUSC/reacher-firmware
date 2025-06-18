#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "Microscope.h"

#define deviceType "MICROSCOPE"
#define eventType "FRAME"

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
  
  json["level"] = 444;
  json["device"] = deviceType;
  json["pin"] = triggerPin;
  json["var"] = "armed";
  json["val"] = this->armed;

  serializeJson(json, Serial);
  Serial.println();
}

void Microscope::SetOffset(uint32_t offset) {
    this->offset = offset;
}

void Microscope::LogOutput() {
  JsonDocument json;

  json["level"] = 777;
  json["device"] = deviceType;
  json["pin"] = timestampPin;
  json["event"] = eventType;
  json["ts"] = instance->timestamp;

  serializeJson(json, Serial);
  Serial.println();   
}

void Microscope::Config(JsonDocument* json) {
  JsonObject conf = json->createNestedObject(deviceType);

  conf["trigger_pin"] = triggerPin;
  conf["timestamp_pin"] = timestampPin;
}
