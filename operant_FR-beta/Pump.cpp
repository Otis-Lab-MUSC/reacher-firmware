#include <Arduino.h>
#include <ArduinoJson.h>

#include "Pump.h"

Pump::Pump(int8_t pin, uint32_t duration, uint32_t traceInterval) : Device(pin, OUTPUT) {
  const char deviceType[] = "PUMP";
  const char eventType[] = "INFUSION";

  this->pin = pin;
  this->duration = duration;
  this->traceInterval = traceInterval;
  armed = false;
  pinMode(pin, OUTPUT);
}

void Pump::ArmToggle(bool armed) {
  doc.clear();
  
  this->armed = armed;
  
  doc["level"] = 444;
  doc["device"] = deviceType;
  doc["pin"] = pin;
  doc["var"] = "armed";
  doc["val"] = this->armed;

  serializeJson(doc, Serial);
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
  doc.clear();
  
  this->duration = duration;

  doc["level"] = 444;
  doc["device"] = deviceType;
  doc["pin"] = pin;
  doc["var"] = "duration";
  doc["val"] = this->duration;

  serializeJson(doc, Serial);
  Serial.println();
}

void Pump::SetTraceInterval(uint32_t traceInterval) {
  doc.clear();
  
  this->traceInterval = traceInterval;
  
  doc["level"] = 444;
  doc["device"] = deviceType;
  doc["pin"] = pin;
  doc["var"] = "traceInterval";
  doc["val"] = this->traceInterval;

  serializeJson(doc, Serial);
  Serial.println();
}

void Pump::On() {
  digitalWrite(pin, HIGH);
}

void Pump::Off() {
  digitalWrite(pin, LOW);
}

void Pump::LogOutput() {
  doc.clear();
  
  doc["level"] = 777;
  doc["device"] = deviceType;
  doc["pin"] = pin;
  doc["event"] = eventType;
  doc["ts1"] = startTimestamp - Offset();
  doc["ts2"] = endTimestamp - Offset();

  serializeJson(doc, Serial);
  Serial.println();  
}

void Pump::Config(JsonDocument* doc) {
  JsonObject conf = doc->createNestedObject(deviceType);

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
