#include <Arduino.h>
#include "Device.h"

#ifndef LASER_H
#define LASER_H

class Laser : public Device {
public:
  Laser(int8_t pin, uint32_t frequency, uint32_t duration, uint32_t traceInterval);
  void ArmToggle(bool armed);
  void Await();

  void SetEvent();
  void SetDuration(uint32_t duration);
  void SetFrequency(uint32_t frequency);

  uint32_t Duration();
  uint32_t Frequency();
  uint32_t TraceInterval();
  
private:
  uint32_t frequency;
  uint32_t duration;
  uint32_t traceInterval;
  uint32_t startTimestamp;
  uint32_t endTimestamp;
  uint32_t halfCycleStartTimestamp;
  uint32_t halfCycleEndTimestamp;
  enum Mode { CONTINGENT, INDEPENDENT };
  Mode mode;
  bool state;
  bool halfState;
  bool outputLogged;

  void On();
  void Off();
  void Cycle(uint32_t currentTimestamp);
  void Oscillate(uint32_t currentTimestamp);
};

#endif // LASER_H
