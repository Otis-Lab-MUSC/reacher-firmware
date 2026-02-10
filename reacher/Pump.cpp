#include "Pump.h"

Pump::Pump(int8_t pin, uint32_t duration)
  : Device(pin, OUTPUT, "PUMP") {
  this->duration = duration;
  startTimestamp = 0;
  endTimestamp = 0;
}

void Pump::Activate(uint32_t startTs, uint32_t dur) {
  startTimestamp = startTs;
  endTimestamp = startTs + dur;
}

void Pump::Await(uint32_t currentTimestamp) {
  if (armed) {
    if (currentTimestamp >= startTimestamp && currentTimestamp <= endTimestamp) {
      On();
    } else {
      Off();
    }
  }
}

void Pump::SetDuration(uint32_t duration) {
  this->duration = duration;
}

uint32_t Pump::Duration() const {
  return duration;
}

void Pump::On() {
  digitalWrite(pin, HIGH);
}

void Pump::Off() {
  digitalWrite(pin, LOW);
}
