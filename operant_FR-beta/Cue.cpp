#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "Cue.h"

#define deviceType "CUE"
#define eventType "TONE"

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
  
  json["level"] = 444;
  json["device"] = deviceType;
  json["pin"] = pin;
  json["var"] = F("armed");
  json["val"] = this->armed;

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

  json["level"] = 444;
  json["device"] = deviceType;
  json["pin"] = pin;
  json["var"] = "frequency";
  json["val"] = this->frequency;

  serializeJson(json, Serial);
  Serial.println();
}

void Cue::SetDuration(uint32_t duration) {
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

void Cue::SetTraceInterval(uint32_t traceInterval) {
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

  json["level"] = 777;
  json["device"] = deviceType;
  json["pin"] = pin;
  json["event"] = eventType;
  json["ts1"] = startTimestamp - Offset();
  json["ts2"] = endTimestamp - Offset();

  serializeJson(json, Serial);
  Serial.println();
}

void Cue::Config(JsonDocument* json) {
  JsonObject conf = json->createNestedObject(deviceType);

  conf["pin"] = pin;
  conf["frequency"] = frequency;
  conf["trace"] = traceInterval;
  conf["duration"] = duration;
}
