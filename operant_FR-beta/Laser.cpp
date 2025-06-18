#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "Laser.h"

#define deviceType "LASER"
#define eventType "STIM"

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
  this->armed = armed;
  
  json["level"] = 444;
  json["device"] = deviceType;
  json["pin"] = pin;
  json["var"] = "armed";
  json["val"] = this->armed;

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
  this->frequency = frequency;

  json["level"] = 444;
  json["device"] = deviceType;
  json["pin"] = pin;
  json["var"] = "frequency";
  json["val"] = this->frequency;

  serializeJson(json, Serial);
  Serial.println();
}

void Laser::SetDuration(uint32_t duration) {
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

void Laser::SetTraceInterval(uint32_t traceInterval) {
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

void Laser::SetMode(bool mode) {
  JsonDocument json;
  
  if (mode) {
    this->mode = CONTINGENT;
  } else {
    this->mode = INDEPENDENT;
  }

  json["level"] = 444;
  json["device"] = deviceType;
  json["pin"] = pin;
  json["var"] = F("mode");
  json["val"] = this->mode;

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

  json["level"] = 777;
  json["device"] = deviceType;
  json["pin"] = pin;
  json["event"] = eventType;
  json["ts1"] = startTimestamp - Offset();
  json["ts2"] = endTimestamp - Offset();

  serializeJson(json, Serial);
  Serial.println();

  outputLogged = true;  
}

void Laser::Config(JsonDocument* json) {
  JsonObject conf = json->createNestedObject(deviceType);

  conf["pin"] = pin;
  conf["frequency"] = frequency;
  conf["trace"] = traceInterval;
  conf["duration"] = duration;
  conf["mode"] = mode;
}
