#include <Arduino.h>
#include <ArduinoJson.h>

#include "Cue.h"

Cue::Cue(int8_t pin, uint32_t frequency, uint32_t duration, uint32_t traceInterval) : Device(pin, OUTPUT) {
  const char deviceType[] = "CUE";
  const char eventType[] = "TONE";
  
  this->pin = pin;
  this->frequency = frequency;
  this->duration = duration;
  this->traceInterval = traceInterval;
  armed = false;
  pinMode(pin, OUTPUT);
}

void Cue::ArmToggle(bool armed) {
  doc.clear();
  
  this->armed = armed;
  
  doc["level"] = 444;
  doc["device"] = deviceType;
  doc["pin"] = pin;
  doc["var"] = "armed";
  doc["val"] = this->armed;

//  SerializeVar("armed", this->armed);

  serializeJson(doc, Serial);
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
  doc.clear();
  
  this->frequency = frequency;

  doc["level"] = 444;
  doc["device"] = deviceType;
  doc["pin"] = pin;
  doc["var"] = "frequency";
  doc["val"] = this->frequency;

  serializeJson(doc, Serial);
  Serial.println();
}

void Cue::SetDuration(uint32_t duration) {
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

void Cue::SetTraceInterval(uint32_t traceInterval) {
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
  
  doc["level"] = 777;
  doc["device"] = deviceType;
  doc["pin"] = pin;
  doc["event"] = eventType;
  doc["ts1"] = startTimestamp - Offset();
  doc["ts2"] = endTimestamp - Offset();

  serializeJson(doc, Serial);
  Serial.println();
}

void Cue::Config(JsonDocument* doc) {
  JsonObject conf = doc->createNestedObject(deviceType);

  conf["pin"] = pin;
  conf["frequency"] = frequency;
  conf["trace"] = traceInterval;
  conf["duration"] = duration;
}
