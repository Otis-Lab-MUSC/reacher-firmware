#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "Laser.h"

Laser::Laser(int8_t pin, uint32_t traceInterval, uint32_t duration) : Device(pin, OUTPUT) {
  this->pin = pin;
  this->traceInterval = traceInterval;
  this->duration = duration;
  armed = false;
  mode = INDEPENDENT;
  state = false;
  pinMode(pin, OUTPUT);
}

void Laser::ArmToggle(bool armed) {
  JsonDocument json;
  String desc;
  this->armed = armed;
  
  desc = F("Laser ");
  desc += armed ? F("armed") : F("disarmed");
  desc += F(" at pin ");
  desc += pin;

  json["level"] = F("info");
  json["desc"] = desc;

  serializeJsonPretty(json, Serial);
  Serial.println();
}

void Laser::Await() {
  uint32_t currentTimestamp = millis();

  if (mode == INDEPENDENT) {
    Cycle(currentTimestamp);
  }
  
  if (armed) {
    if (currentTimestamp >= startTimestamp && currentTimestamp <= endTimestamp) {
      On();
    } else {
      Off();
    }
  }
}

void Laser::SetEvent() {
  JsonDocument json;
  String desc;

  if (armed && (mode == CONTINGENT)) {
    startTimestamp = traceInterval + millis();
    endTimestamp = startTimestamp + duration;
    
    desc = F("Laser stimulation occurring at pin ");;
    desc += pin;
  
    json["level"] = F("output");
    json["desc"] = desc;
    json["device"] = F("LASER");
    json["start_timestamp"] = startTimestamp;
    json["end_timestamp"] = endTimestamp;
  
    serializeJsonPretty(json, Serial);
    Serial.println();
  }  
}

void Laser::SetDuration(uint32_t duration) {
  this->duration = duration;
}

void Laser::SetFrequency(uint32_t frequency) {
  this->traceInterval = traceInterval;
}

uint32_t Laser::Duration() {
  return duration; 
}

uint32_t Laser::Frequency() {
  return frequency; 
}

uint32_t Laser::TraceInterval() {
  return traceInterval;
}

void Laser::On() {
//  digitalWrite(pin, HIGH);
  Serial.print(F("*"));
  Serial.println(state);
}

void Laser::Off() {
//  digitalWrite(pin, LOW);
}

void Laser::Cycle(uint32_t currentTimestamp) {
  if (currentTimestamp >= endTimestamp) {
    startTimestamp = currentTimestamp;
    endTimestamp = currentTimestamp + duration;
    state = !state;
  }
}

void Laser::Oscillate() {
  
}
