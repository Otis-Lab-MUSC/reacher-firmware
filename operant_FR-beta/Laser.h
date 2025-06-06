#include <Arduino.h>
#include "Device.h"

#ifndef LASER_H
#define LASER_H

class Laser : public Device {
public:
  Laser(int8_t pin, uint32_t traceInterval, uint32_t duration);
  void ArmToggle(bool armed);
  void Await();

  void SetEvent();
  void SetDuration(uint32_t duration);
  void SetFrequency(uint32_t frequency);

  uint32_t Duration();
  uint32_t Frequency();
  uint32_t TraceInterval();
  
private:
  uint32_t duration;
  uint32_t frequency;
  uint32_t traceInterval;
  uint32_t startTimestamp;
  uint32_t endTimestamp;
  enum Mode { CONTINGENT, INDEPENDENT };
  Mode mode;
  bool state;

  void On();
  void Off();
  void Cycle(uint32_t currentTimestamp);
  void Oscillate();
};

#endif // LASER_H
