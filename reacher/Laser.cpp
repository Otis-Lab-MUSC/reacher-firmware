#include "Laser.h"

Laser::Laser(int8_t pin, uint32_t frequency, uint32_t duration)
  : Device(pin, OUTPUT, "LASER") {
  this->frequency = frequency;
  this->duration = duration;
  startTimestamp = 0;
  endTimestamp = 0;
  halfCycleStartTimestamp = 0;
  halfCycleEndTimestamp = 0;
  mode = CONTINGENT;
  state = false;
  halfState = false;
  isTesting = false;
}

void Laser::Activate(uint32_t startTs, uint32_t dur) {
  if (mode == CONTINGENT) {
    startTimestamp = startTs;
    endTimestamp = startTs + dur;
    state = true;
    UpdateHalfCycle(startTs);
  }
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

void Laser::Test(uint32_t currentTimestamp) {
  startTimestamp = currentTimestamp;
  endTimestamp = currentTimestamp + duration;
  state = true;
  UpdateHalfCycle(startTimestamp);
  isTesting = true;
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
}

void Laser::SetFrequency(uint32_t frequency) {
  if (frequency > 0) {
    this->frequency = frequency;
  }
}

void Laser::SetDuration(uint32_t duration) {
  this->duration = duration;
}

void Laser::SetMode(bool contingent) {
  mode = contingent ? CONTINGENT : INDEPENDENT;
}

uint32_t Laser::Frequency() const {
  return frequency;
}

uint32_t Laser::Duration() const {
  return duration;
}

bool Laser::IsContingent() const {
  return mode == CONTINGENT;
}

void Laser::On() {
  digitalWrite(pin, HIGH);
}

void Laser::Off() {
  digitalWrite(pin, LOW);
  halfState = false;
}

void Laser::UpdateHalfCycle(uint32_t currentTimestamp) {
  float halfCycleLength = (1.0f / frequency) / 2.0f * 1000.0f;
  halfCycleStartTimestamp = currentTimestamp;
  halfCycleEndTimestamp = currentTimestamp + static_cast<uint32_t>(halfCycleLength);
  halfState = !halfState;
}
