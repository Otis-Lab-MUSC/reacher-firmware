#include <Arduino.h>
#include "Device.h"

#ifndef CUE_H
#define CUE_H

class Cue : public Device {
public:
  Cue(int8_t pin, uint32_t frequency, uint32_t duration);
  void ArmToggle(bool armed);
  void Await();
  void Jingle();

  void SetEvent();
  void SetFrequency(uint32_t frequency);
  void SetDuration(uint32_t duration);

  uint32_t Frequency();
  uint32_t Duration();
  
private:
  uint32_t frequency;
  uint32_t duration;
  uint32_t startTimestamp;
  uint32_t endTimestamp;

  void On();
  void Off();
};

#endif // CUE_H
