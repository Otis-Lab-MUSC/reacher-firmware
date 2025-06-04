#include <Arduino.h>
#include "Device.h"

#ifndef LASER_H
#define LASER_H

class Laser : public Device {
public:
  Laser(int8_t pin);
  void ArmToggle(bool armed);
  void Await();

  void SetEvent();
  void SetDuration(uint32_t duration);
  void SetFrequency(uint32_t frequency);

  uint32_t Duration();
  uint32_t Frequency();
  
private:
  uint32_t duration;
  uint32_t frequency;
  uint32_t startTimestamp;
  uint32_t endTimestamp;
  uint32_t cycleStartTimestamp;
  uint32_t cycleEndTimestamp;
  bool cycle;
  bool state;
  bool action;
  enum Mode { CYCLE, ACTIVE_PRESS };
  Mode mode;

  void Stim(uint32_t currentTimestamp);
  void On();
  void Off();
};

#endif // LASER_H
