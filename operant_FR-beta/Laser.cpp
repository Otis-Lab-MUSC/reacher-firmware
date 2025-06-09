#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "Laser.h"

Laser::Laser(int8_t pin, uint32_t frequency, uint32_t duration, uint32_t traceInterval) : Device(pin, OUTPUT) {
  this->pin = pin;
  this->frequency = frequency;
  this->duration = duration;
  this->traceInterval = traceInterval;
  armed = false;
  mode = INDEPENDENT;
  state = false;
  halfState = false;
  outputLogged = false;
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
    Oscillate(currentTimestamp);
  }
}

void Laser::SetEvent() {
  JsonDocument json;
  String desc;
  uint32_t currentTimestamp = millis();

  if (armed ) {
    if (mode == CONTINGENT) {
      startTimestamp = traceInterval + currentTimestamp;
      endTimestamp = startTimestamp + duration;
    }
    else if (mode == INDEPENDENT) {
      float halfCycleLength = (1.0f / frequency) / 2.0f * 1000.0f;
      
      halfCycleStartTimestamp = currentTimestamp;
      halfCycleEndTimestamp = currentTimestamp + static_cast<uint32_t>(halfCycleLength);  
      halfState = !halfState;       
    }

    if (!outputLogged) { // FIXME: not being called correctly...repeated calls instead of once
      desc = F("Laser stimulation occurring at pin ");;
      desc += pin;
    
      json["level"] = F("output");
      json["desc"] = desc;
      json["device"] = F("LASER");
      json["start_timestamp"] = startTimestamp;
      json["end_timestamp"] = endTimestamp;
    
      serializeJsonPretty(json, Serial);
      Serial.println();
  
      outputLogged = true;
    }
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
//  Serial.print(F("* "));
//  Serial.println(halfState);
}

void Laser::Off() {
//  digitalWrite(pin, LOW);
//  Serial.print(F("* "));
//  Serial.println(halfState);
}

void Laser::Cycle(uint32_t currentTimestamp) {
  if (currentTimestamp >= endTimestamp) {
    startTimestamp = currentTimestamp;
    endTimestamp = currentTimestamp + duration;
    state = !state;
  }
}

void Laser::Oscillate(uint32_t currentTimestamp) {
  if (currentTimestamp >= startTimestamp && currentTimestamp <= endTimestamp && state) {
    if (frequency == 1) {
      On();
    }
    else {
//      float halfCycleLength = (1.0f / frequency) / 2.0f * 1000.0f;
      if (currentTimestamp >= halfCycleEndTimestamp) {
//        halfCycleStartTimestamp = currentTimestamp;
//        halfCycleEndTimestamp = currentTimestamp + static_cast<uint32_t>(halfCycleLength);  
//        halfState = !halfState;    
          SetEvent();  
      }
      if (halfState) {
        On();
      }
      else {
        Off();
      }
    }
  }
  else {
    Off();
  }
}
