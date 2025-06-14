#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "Pump.h"

Pump::Pump(int8_t pin, uint32_t duration, uint32_t traceInterval) : Device(pin, OUTPUT) {
  this->pin = pin;
  this->duration = duration;
  this->traceInterval = traceInterval;
  armed = false;
  pinMode(pin, OUTPUT);
}

void Pump::ArmToggle(bool armed) {
  JsonDocument json;
  String desc;
  this->armed = armed;
  
  desc = F("Pump ");
  desc += armed ? F("armed") : F("disarmed");
  desc += F(" at pin ");
  desc += pin;

  json["level"] = F("PROGINFO");
  json["desc"] = desc;

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
  String desc;
  this->duration = duration;
  
  desc = F("Pump duration set to ");
  desc += this->duration;
  desc += F("ms for pump at pin ");
  desc += pin;

  json["level"] = F("PROGINFO");
  json["desc"] = desc;

  serializeJson(json, Serial);
  Serial.println();}

void Pump::SetTraceInterval(uint32_t traceInterval) {
  JsonDocument json;
  String desc;
  this->traceInterval = traceInterval;
  
  desc = F("Pump trace interval set to ");
  desc += this->traceInterval;
  desc += F("ms for pump at pin ");
  desc += pin;

  json["level"] = F("PROGINFO");
  json["desc"] = desc;

  serializeJson(json, Serial);
  Serial.println();}

void Pump::On() {
  digitalWrite(pin, HIGH);
}

void Pump::Off() {
  digitalWrite(pin, LOW);
}

void Pump::LogOutput() {
  JsonDocument json;
  String desc;
  
  desc = F("Pump infusion occurring at pin ");;
  desc += pin;

  json["level"] = F("PROGOUT");
  json["desc"] = desc;
  json["device"] = F("PUMP");
  json["start_timestamp"] = startTimestamp - Offset();
  json["end_timestamp"] = endTimestamp - Offset();
  json["duration"] = duration;
  json["trace"] = traceInterval;
  json["offset"] = Offset();

  serializeJson(json, Serial);
  Serial.println();  
}

uint32_t Pump::Duration() {
  return duration;
}

uint32_t Pump::TraceInterval() {
  return traceInterval;
}
