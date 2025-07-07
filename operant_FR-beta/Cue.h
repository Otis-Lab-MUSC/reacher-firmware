#include <Arduino.h>
#include "Device.h"

#ifndef CUE_H
#define CUE_H

class Cue : public Device {
public:
  Cue(int8_t pin, uint32_t frequency, uint32_t duration, uint32_t traceInterval);
  
  void Await(uint32_t currentTimestamp);
  void Jingle();

  void SetEvent(uint32_t currentTimestamp);
  void SetFrequency(uint32_t frequency);
  void SetDuration(uint32_t duration);
  void SetTraceInterval(uint32_t traceInterval);

  uint32_t Frequency();
  uint32_t Duration();
  uint32_t TraceInterval();
  
private:
  uint32_t frequency;
  uint32_t duration;
  uint32_t traceInterval;
  uint32_t startTimestamp;
  uint32_t endTimestamp;

  void On();
  void Off();
  void LogOutput();
};

#endif // CUE_H
