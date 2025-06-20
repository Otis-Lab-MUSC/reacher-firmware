#include <Arduino.h>
#include <ArduinoJson.h>

#include "Laser.h"

Laser::Laser(int8_t pin, uint32_t frequency, uint32_t duration, uint32_t traceInterval) : Device(pin, OUTPUT) {
  const char deviceType[] = "LASER";
  const char eventType[] = "STIM";

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
  this->armed = armed;
  
  doc["level"] = 444;
  doc["device"] = deviceType;
  doc["pin"] = pin;
  doc["var"] = "armed";
  doc["val"] = this->armed;

  serializeJson(doc, Serial);
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
  this->frequency = frequency;

  doc["level"] = 444;
  doc["device"] = deviceType;
  doc["pin"] = pin;
  doc["var"] = "frequency";
  doc["val"] = this->frequency;

  serializeJson(doc, Serial);
  Serial.println();
}

void Laser::SetDuration(uint32_t duration) {
  this->duration = duration;

  doc["level"] = 444;
  doc["device"] = deviceType;
  doc["pin"] = pin;
  doc["var"] = "duration";
  doc["val"] = this->duration;

  serializeJson(doc, Serial);
  Serial.println();
}

void Laser::SetTraceInterval(uint32_t traceInterval) {
  this->traceInterval = traceInterval;
  
  doc["level"] = 444;
  doc["device"] = deviceType;
  doc["pin"] = pin;
  doc["var"] = "traceInterval";
  doc["val"] = this->traceInterval;

  serializeJson(doc, Serial);
  Serial.println();
}

void Laser::SetMode(bool mode) { 
  if (mode) {
    this->mode = CONTINGENT;
  } else {
    this->mode = INDEPENDENT;
  }

  doc["level"] = 444;
  doc["device"] = deviceType;
  doc["pin"] = pin;
  doc["var"] = F("mode");
  doc["val"] = this->mode;

  serializeJson(doc, Serial);
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
  doc.clear();
  
  doc["level"] = 777;
  doc["device"] = deviceType;
  doc["pin"] = pin;
  doc["event"] = eventType;
  doc["ts1"] = startTimestamp - Offset();
  doc["ts2"] = endTimestamp - Offset();

  serializeJson(doc, Serial);
  Serial.println();

  outputLogged = true;  
}

void Laser::Config(JsonDocument* doc) {
  JsonObject conf = doc->createNestedObject(deviceType);

  conf["pin"] = pin;
  conf["frequency"] = frequency;
  conf["trace"] = traceInterval;
  conf["duration"] = duration;
  conf["mode"] = mode;
}
