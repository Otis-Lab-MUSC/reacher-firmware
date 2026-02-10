#include "Cue.h"

Cue::Cue(int8_t pin, uint32_t frequency, uint32_t duration)
  : Device(pin, OUTPUT, "CUE") {
  this->frequency = frequency;
  this->duration = duration;
  startTimestamp = 0;
  endTimestamp = 0;
}

void Cue::Activate(uint32_t startTs, uint32_t dur) {
  startTimestamp = startTs;
  endTimestamp = startTs + dur;
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
  int32_t pitch = 500;
  for (int32_t i = 0; i < 3; i++) {
    tone(pin, pitch, 100);
    delay(100);
    noTone(pin);
    pitch += 500;
  }
}

void Cue::SetFrequency(uint32_t frequency) {
  this->frequency = frequency;
}

void Cue::SetDuration(uint32_t duration) {
  this->duration = duration;
}

uint32_t Cue::Frequency() const {
  return frequency;
}

uint32_t Cue::Duration() const {
  return duration;
}

void Cue::On() {
  tone(pin, frequency);
}

void Cue::Off() {
  noTone(pin);
}
