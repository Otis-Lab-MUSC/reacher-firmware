#include <Arduino.h>
#include <ArduinoJson.h>
#include "Laser.h"

Laser::Laser(int8_t pin, uint32_t frequency, uint32_t duration, uint32_t traceInterval) : Device(pin, OUTPUT, "LASER", "STIM") {
  this->pin = pin;
  this->frequency = frequency;
  this->duration = duration;
  this->traceInterval = traceInterval;
  mode = CONTINGENT;
  state = false;
  prevState = false;
  halfState = false;
  outputLogged = false;
  isTesting = false;
  pinMode(pin, OUTPUT);
}

void Laser::Await(uint32_t currentTimestamp) {
  if (armed || isTesting) {
    if (mode == INDEPENDENT && !isTesting) {
      Cycle(currentTimestamp);  
    }
    Oscillate(currentTimestamp); 
  } else {
    startTimestamp = currentTimestamp;
    endTimestamp = currentTimestamp;
    Off();
  }
}

void Laser::Cycle(uint32_t currentTimestamp) {
  if (currentTimestamp >= endTimestamp) {
    if (state) {
      LogOutput();
    }
    startTimestamp = currentTimestamp;
    endTimestamp = currentTimestamp + duration;
    state = !state;
  }
}

void Laser::Oscillate(uint32_t currentTimestamp) {
    if (currentTimestamp >= startTimestamp && currentTimestamp <= endTimestamp && state) {
        if (frequency == 1) {
            On();
        } else {
            if (currentTimestamp >= halfCycleEndTimestamp) {
                UpdateHalfCycle(currentTimestamp);
            }
            if (halfState) {
                On();
            } else {
                Off();
            }
        }
    } else {
        Off();
        if (state && currentTimestamp > endTimestamp) {
            state = false;
        }
        if (isTesting && currentTimestamp > endTimestamp) {
            isTesting = false;
        }
    }
    prevState = state;
}

void Laser::Test(uint32_t currentTimestamp) {
  startTimestamp = currentTimestamp;
  endTimestamp = currentTimestamp + duration;
  state = true;
  UpdateHalfCycle(startTimestamp);
  isTesting = true;
  outputLogged = false;
}

void Laser::SetEvent(uint32_t currentTimestamp) {
  if (armed) {
    if (mode == CONTINGENT) {
      startTimestamp = traceInterval + currentTimestamp;
      endTimestamp = startTimestamp + duration;
      state = true;
      UpdateHalfCycle(startTimestamp);
      LogOutput();
    }
  }
}

void Laser::SetFrequency(uint32_t frequency) {
  this->frequency = frequency;
}

void Laser::SetDuration(uint32_t duration) {
  this->duration = duration;
}

void Laser::SetTraceInterval(uint32_t traceInterval) {
  this->traceInterval = traceInterval;
}

void Laser::SetMode(bool mode) { 
  if (mode) {
    this->mode = CONTINGENT;
  } else {
    this->mode = INDEPENDENT;
  }
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

void Laser::LogOutput() { 
  JsonDocument doc;
   
  doc[F("level")] = F("007");
  doc[F("device")] = device;
  doc[F("pin")] = pin;
  doc[F("event")] = event;
  doc[F("start_timestamp")] = startTimestamp - Offset(); 
  doc[F("end_timestamp")] = endTimestamp - Offset();

  serializeJson(doc, Serial);
  Serial.println();

  outputLogged = true;  
}

void Laser::UpdateHalfCycle(uint32_t currentTimestamp) {
  float halfCycleLength = (1.0f / frequency) / 2.0f * 1000.0f; // Half-cycle in ms
  halfCycleStartTimestamp = currentTimestamp;
  halfCycleEndTimestamp = currentTimestamp + static_cast<uint32_t>(halfCycleLength);
  halfState = !halfState;
}

JsonDocument Laser::Defaults() {
  JsonDocument defaults;

  defaults[F("level")] = F("000"); 
  defaults[F("device")] = device;
  defaults[F("pin")] = pin;
  defaults[F("frequency")] = frequency;
  defaults[F("duration")] = duration;
  defaults[F("trace")] = traceInterval;
  defaults[F("mode")] = (mode == CONTINGENT) ? F("CONTINGENT") : F("INDEPENDENT");

  return defaults;
}
