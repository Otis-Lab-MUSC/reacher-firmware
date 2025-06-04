#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "Laser.h"

Laser::Laser(int8_t pin) : Device(pin, OUTPUT) {
  this->pin = pin;
  armed = false;
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

  if (armed) {
    if (mode == CYCLE) {
      
    }
    else if (mode == ACTIVE_PRESS) {
      
    }
  }
  // stim()
}

void Laser::SetEvent() {
  
}

void Laser::SetDuration(uint32_t duration) {
  
}

void Laser::SetFrequency(uint32_t frequency) {
  
}

uint32_t Laser::Duration() {
  return duration; 
}

uint32_t Laser::Frequency() {
  return frequency; 
}

void Laser::Stim(uint32_t currentTimestamp) {
  if ((currentTimestamp > startTimestamp) && (currentTimestamp < endTimestamp) && cycle) {
    state = true;
    if (frequency == 1) {
      action = true;  
    }
    else {
      if (currentTimestamp > cycleEndTimestamp) {
        cycleStartTimestamp = currentTimestamp;
        cycleEndTimestamp = 0;
        action = (action == true ? false : true);
      }
    }
  }
}

void Laser::On() {
  digitalWrite(pin, HIGH);
}

void Laser::Off() {
  digitalWrite(pin, LOW);
}







//class Laser : public Device {
//public:
//  Laser(int8_t pin);
//  void ArmToggle(bool armed);
//  void Await();
//
//  void SetEvent();
//  void SetDuration(uint32_t duration);
//  void SetFrequency(uint32_t frequency);
//
//  uint32_t Duration();
//  uint32_t Frequency();
//  
//private:
//  uint32_t duration;
//  uint32_t frequency;
//  uint32_t startTimestamp;
//  uint32_t endTimestamp;
//  uint32_t cycleStartTimestamp;
//  uint32_t cycleEndTimestamp;
//
//  void On();
//  void Off();
//};
