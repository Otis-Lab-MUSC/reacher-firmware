#ifndef CUE_H
#define CUE_H

#include <Arduino.h>
#include "Device.h"

class Cue : public Device {
public:
  Cue(int8_t pin, uint32_t frequency, uint32_t duration);

  void Activate(uint32_t startTs, uint32_t dur);
  void Await(uint32_t currentTimestamp);
  void Jingle();

  void SetFrequency(uint32_t frequency);
  void SetDuration(uint32_t duration);

  uint32_t Frequency() const;
  uint32_t Duration() const;

private:
  uint32_t frequency;
  uint32_t duration;
  uint32_t startTimestamp;
  uint32_t endTimestamp;

  void On();
  void Off();
};

#endif // CUE_H
