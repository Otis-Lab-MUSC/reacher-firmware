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

  json["level"] = F("PROGINFO");
  json["desc"] = desc;

  serializeJson(json, Serial);
  Serial.println();
}

void Laser::Await(uint32_t currentTimestamp) {
  if (mode == INDEPENDENT) {
    Cycle(currentTimestamp);
  }
  
  if (armed) {
    Oscillate(currentTimestamp);
  } else {
    startTimestamp = currentTimestamp;
    endTimestamp = currentTimestamp;
    Off();
  }
}

void Laser::SetEvent(uint32_t currentTimestamp) {
  if (armed) {
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
  
    if (!outputLogged) {
      LogOutput();
    }
  }
}

void Laser::SetFrequency(uint32_t frequency) {
  JsonDocument json;
  String desc;
  this->frequency = frequency;
  
  desc = F("Laser frequency set to ");
  desc += this->duration;
  desc += F("Hz for laser at pin ");
  desc += pin;

  json["level"] = F("PROGINFO");
  json["desc"] = desc;

  serializeJson(json, Serial);
  Serial.println();
}

void Laser::SetDuration(uint32_t duration) {
  JsonDocument json;
  String desc;
  this->duration = duration;
  
  desc = F("Laser duration set to ");
  desc += this->duration;
  desc += F("ms for laser at pin ");
  desc += pin;

  json["level"] = F("PROGINFO");
  json["desc"] = desc;

  serializeJson(json, Serial);
  Serial.println();
}

void Laser::SetTraceInterval(uint32_t traceInterval) {
  JsonDocument json;
  String desc;
  this->traceInterval = traceInterval;
  
  desc = F("Laser trace interval set to ");
  desc += this->traceInterval;
  desc += F("ms for laser at pin ");
  desc += pin;

  json["level"] = F("PROGINFO");
  json["desc"] = desc;

  serializeJson(json, Serial);
  Serial.println();
}

void Laser::SetMode(bool mode) {
  JsonDocument json;
  String desc;
  
  if (mode) {
    this->mode = CONTINGENT;
  } else {
    this->mode = INDEPENDENT;
  }

  desc = F("Laser mode set to ");
  desc += (mode) ? F("CONTINGENT") : F("INDEPENDENT");
  desc += F(" for laser at pin ");
  desc += pin;

  json["level"] = F("PROGINFO");
  json["desc"] = desc;

  serializeJson(json, Serial);
  Serial.println();
}

uint32_t Laser::Frequency() {
  return frequency; 
}

uint32_t Laser::Duration() {
  return duration; 
}

uint32_t Laser::TraceInterval() {
  return traceInterval;
}

void Laser::On() {
  digitalWrite(pin, HIGH);
}

void Laser::Off() {
  digitalWrite(pin, LOW);
  halfState = false;
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
      if (currentTimestamp >= halfCycleEndTimestamp) { 
          SetEvent(currentTimestamp);  
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
    outputLogged = false;
  }
}

void Laser::LogOutput() {
  JsonDocument json;
  String desc;
  
  desc = F("Laser stimulation occurring at pin ");;
  desc += pin;

  json["level"] = F("PROGOUT");
  json["desc"] = desc;
  json["device"] = F("LASER");
  json["start_timestamp"] = startTimestamp - Offset();
  json["end_timestamp"] = endTimestamp - Offset();
  json["frequency"] = frequency;
  json["duration"] = duration;
  json["trace"] = traceInterval;
  json["mode"] = (mode == INDEPENDENT) ? F("INDEPENDENT") : F("CONTINGENT");
  json["offset"] = Offset();

  serializeJson(json, Serial);
  Serial.println();

  outputLogged = true;  
}
