#include <Arduino.h>
#include <ArduinoJson.h>

#include "Cue.h"

Cue::Cue(int8_t pin, uint32_t frequency, uint32_t duration, uint32_t traceInterval) : Device(pin, OUTPUT, "CUE", "TONE") {
  this->pin = pin;
  this->frequency = frequency;
  this->duration = duration;
  this->traceInterval = traceInterval;
  pinMode(pin, OUTPUT);
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
  this->frequency = frequency;
}

void Cue::SetDuration(uint32_t duration) {
  this->duration = duration;
}

void Cue::SetTraceInterval(uint32_t traceInterval) {
  this->traceInterval = traceInterval;
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
