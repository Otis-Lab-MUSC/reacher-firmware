#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "Pump.h"

#define deviceType "PUMP"
#define eventType "INFUSION"

Pump::Pump(int8_t pin, uint32_t duration, uint32_t traceInterval) : Device(pin, OUTPUT) {
  this->pin = pin;
  this->duration = duration;
  this->traceInterval = traceInterval;
  armed = false;
  pinMode(pin, OUTPUT);
}

void Pump::ArmToggle(bool armed) {
  JsonDocument json;
  this->armed = armed;
  
  json["level"] = 444;
  json["device"] = deviceType;
  json["pin"] = pin;
  json["var"] = "armed";
  json["val"] = this->armed;

  serializeJson(json, Serial);
  Serial.println();
}

void Pump::Await(uint32_t currentTimestamp) {
  if (armed) {
    if (currentTimestamp >= startTimestamp && currentTimestamp <= endTimestamp) {
      On();
    } else {
      Off();
    }
  }
}

void Pump::SetEvent(uint32_t currentTimestamp) {  
  if (armed) {
    startTimestamp = traceInterval + currentTimestamp;
    endTimestamp = startTimestamp + duration;
    
    LogOutput();
  }
}

void Pump::SetDuration(uint32_t duration) {
  JsonDocument json;
  this->duration = duration;

  json["level"] = 444;
  json["device"] = deviceType;
  json["pin"] = pin;
  json["var"] = "duration";
  json["val"] = this->duration;

  serializeJson(json, Serial);
  Serial.println();
}

void Pump::SetTraceInterval(uint32_t traceInterval) {
  JsonDocument json;
  this->traceInterval = traceInterval;
  
  json["level"] = 444;
  json["device"] = deviceType;
  json["pin"] = pin;
  json["var"] = "traceInterval";
  json["val"] = this->traceInterval;

  serializeJson(json, Serial);
  Serial.println();
}

void Pump::On() {
  digitalWrite(pin, HIGH);
}

void Pump::Off() {
  digitalWrite(pin, LOW);
}

void Pump::LogOutput() {
  JsonDocument json;

  json["level"] = 777;
  json["device"] = deviceType;
  json["pin"] = pin;
  json["event"] = eventType;
  json["ts1"] = startTimestamp - Offset();
  json["ts2"] = endTimestamp - Offset();

  serializeJson(json, Serial);
  Serial.println();  
}

void Pump::Config(JsonDocument* json) {
  JsonObject conf = json->createNestedObject(deviceType);

  conf["pin"] = pin;
  conf["trace"] = traceInterval;
  conf["duration"] = duration;
}

uint32_t Pump::Duration() {
  return duration;
}

uint32_t Pump::TraceInterval() {
  return traceInterval;
}
