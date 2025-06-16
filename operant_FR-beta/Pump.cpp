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
  this->armed = armed;
  
  json["level"] = F("PROGINFO");
  json["device"] = F("PUMP");
  json["pin"] = pin;
  json["desc"] = armed ? F("Pump armed") : F("Pump disarmed");

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

  json["level"] = F("PROGINFO");
  json["device"] = F("PUMP");
  json["pin"] = pin;
  json["duration"] = this->duration;
  json["desc"] = F("Duration changed");

  serializeJson(json, Serial);
  Serial.println();
}

void Pump::SetTraceInterval(uint32_t traceInterval) {
  JsonDocument json;
  this->traceInterval = traceInterval;
  
  json["level"] = F("PROGINFO");
  json["device"] = F("PUMP");
  json["pin"] = pin;
  json["trace"] = this->traceInterval;
  json["desc"] = F("Trace interval changed");

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

  json["level"] = F("PROGOUT");
  json["device"] = F("PUMP");
  json["pin"] = pin;
  json["event"] = F("INFUSION");
  json["start_timestamp"] = startTimestamp - Offset();
  json["end_timestamp"] = endTimestamp - Offset();
  json["desc"] = F("Pump infusion occurred");

  serializeJson(json, Serial);
  Serial.println();  
}

uint32_t Pump::Duration() {
  return duration;
}

uint32_t Pump::TraceInterval() {
  return traceInterval;
}
