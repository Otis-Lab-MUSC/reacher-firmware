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
  this->armed = armed;
  
  json["level"] = F("PROGINFO");
  json["device"] = F("CUE");
  json["pin"] = pin;
  json["desc"] = armed ? F("Cue armed") : F("Cue disarmed");

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
  this->frequency = frequency;

  json["level"] = F("PROGINFO");
  json["device"] = F("CUE");
  json["pin"] = pin;
  json["frequency"] = this->frequency;
  json["desc"] = F("Frequency changed");

  serializeJson(json, Serial);
  Serial.println();
}

void Cue::SetDuration(uint32_t duration) {
  JsonDocument json;
  this->duration = duration;

  json["level"] = F("PROGINFO");
  json["device"] = F("CUE");
  json["pin"] = pin;
  json["duration"] = this->duration;
  json["desc"] = F("Duration changed");

  serializeJson(json, Serial);
  Serial.println();
}

void Cue::SetTraceInterval(uint32_t traceInterval) {
  JsonDocument json;
  this->traceInterval = traceInterval;
  
  json["level"] = F("PROGINFO");
  json["device"] = F("CUE");
  json["pin"] = pin;
  json["trace"] = this->traceInterval;
  json["desc"] = F("Trace interval changed");

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

  json["level"] = F("PROGOUT");
  json["device"] = F("CUE");
  json["pin"] = pin;
  json["event"] = F("TONE");
  json["start_timestamp"] = startTimestamp - Offset();
  json["end_timestamp"] = endTimestamp - Offset();
//  json["frequency"] = frequency;
//  json["duration"] = duration;
//  json["trace"] = traceInterval;
//  json["offset"] = Offset();
  json["desc"] = F("Cue tone occurred");

  serializeJson(json, Serial);
  Serial.println();
}
