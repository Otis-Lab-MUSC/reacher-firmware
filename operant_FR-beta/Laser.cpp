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
    if (true) {
      On();
    } else {
      Off();
    }
  }
}









//class Laser : public Device {
//public:
//  Pump(int8_t pin, uint32_t traceInterval, uint32_t duration);
//  void ArmToggle(bool armed);
//  void Await();
//
//  void SetEvent();
//  void SetDuration(uint32_t duration);
//  void SetFrequency(uint32_t frequency);
//  void SetMode(Mode mode);
//
//  uint32_t Duration();
//  uint32_t Frequency();
//  enum Mode { CYCLE, ACTIVE_PRESS };
//  Mode mode;
//  
//private:
//  uint32_t traceInterval;
//  uint32_t duration;
//  uint32_t startTimestamp;
//  uint32_t endTimestamp;
//  enum State { INACTIVE, ACTIVE };
//  State state;
//  enum Action { OFF, ON };
//  Action action;
//  
//  void On();
//  void Off();
//
//};
