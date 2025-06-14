#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "Cue.h"

Cue::Cue(int8_t pin, uint32_t frequency, uint32_t duration, uint32_t traceInterval) : Device(pin, OUTPUT) {
  this->pin = pin;
  this->frequency = frequency;
  this->duration = duration;
  this->traceInterval = traceInterval;
  armed = false;
  pinMode(pin, OUTPUT);
}

void Cue::ArmToggle(bool armed) {
  JsonDocument json;
  String desc;
  this->armed = armed;
  
  desc = F("Cue ");
  desc += armed ? F("armed") : F("disarmed");
  desc += F(" at pin ");
  desc += pin;

  json["level"] = F("PROGINFO");
  json["desc"] = desc;

  serializeJson(json, Serial);
  Serial.println();
}

void Cue::Await(uint32_t currentTimestamp) {
  if (armed) {
    if (currentTimestamp >= startTimestamp && currentTimestamp <= endTimestamp) {
      On();
    } else {
      Off();
    }
  }
}

void Cue::Jingle() {
  static int32_t pitch = 500; 
  uint32_t duration = 100;
  for (int32_t i = 0; i < 3; i++) {
      tone(pin, pitch, 100); 
      delay(100);                   
      noTone(pin);           
      pitch += 500;             
  }
}

void Cue::SetEvent(uint32_t currentTimestamp) {
  if (armed) {
    startTimestamp = currentTimestamp;
    endTimestamp = startTimestamp + duration;

    LogOutput();
  }
}

void Cue::SetFrequency(uint32_t frequency) {
  JsonDocument json;
  String desc;
  this->frequency =  frequency;

  desc = F("Cue frequency set to ");
  desc += this->frequency;
  desc += F("Hz for cue at pin ");
  desc += pin;

  json["level"] = F("PROGINFO");
  json["desc"] = desc;

  serializeJson(json, Serial);
  Serial.println();
}

void Cue::SetDuration(uint32_t duration) {
  JsonDocument json;
  String desc;
  this->duration = duration;
  
  desc = F("Cue duration set to ");
  desc += this->duration;
  desc += F("ms for cue at pin ");
  desc += pin;

  json["level"] = F("PROGINFO");
  json["desc"] = desc;

  serializeJson(json, Serial);
  Serial.println();
}

void Cue::SetTraceInterval(uint32_t traceInterval) {
  JsonDocument json;
  String desc;
  this->traceInterval = traceInterval;
  
  desc = F("Cue trace interval set to ");
  desc += this->traceInterval;
  desc += F("ms for cue at pin ");
  desc += pin;

  json["level"] = F("PROGINFO");
  json["desc"] = desc;

  serializeJson(json, Serial);
  Serial.println();
}

uint32_t Cue::Frequency() {
  return frequency;
}

uint32_t Cue::Duration() {
  return duration;
}

uint32_t Cue::TraceInterval() {
  return traceInterval;
}

void Cue::On() {
  tone(pin, frequency);
}

void Cue::Off() {
  noTone(pin);
}

void Cue::LogOutput() {
  JsonDocument json;
  String desc;
  
  desc = F("Cue tone occurring at pin ");;
  desc += pin;

  json["level"] = F("PROGOUT");
  json["desc"] = desc;
  json["device"] = F("CUE");
  json["start_timestamp"] = startTimestamp - Offset();
  json["end_timestamp"] = endTimestamp - Offset();
  json["frequency"] = frequency;
  json["duration"] = duration;
  json["trace"] = traceInterval;
  json["offset"] = Offset();

  serializeJson(json, Serial);
  Serial.println();
}
