#include <Arduino.h>
#include <ArduinoJson.h>

#include "Pump.h"

Pump::Pump(int8_t pin, uint32_t duration, uint32_t traceInterval) : Device(pin, OUTPUT, "PUMP", "INFUSION") {
  this->pin = pin;
  this->duration = duration;
  this->traceInterval = traceInterval;
  pinMode(pin, OUTPUT);
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
  this->duration = duration;
}

void Pump::SetTraceInterval(uint32_t traceInterval) {
  this->traceInterval = traceInterval;
}

void Pump::On() {
  digitalWrite(pin, HIGH);
}

void Pump::Off() {
  digitalWrite(pin, LOW);
}

void Pump::LogOutput() {
  doc.clear();
  
  doc["level"] = F("007");
  doc["device"] = device;
  doc["pin"] = pin;
  doc["event"] = event;
  doc["start_timestamp"] = startTimestamp - Offset();
  doc["end_timestamp"] = endTimestamp - Offset();

  serializeJson(doc, Serial);
  Serial.println();  
}

uint32_t Pump::Duration() {
  return duration;
}

uint32_t Pump::TraceInterval() {
  return traceInterval;
}
