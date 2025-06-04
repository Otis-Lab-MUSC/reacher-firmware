#include <Arduino.h>
#include "Device.h"

#ifndef PUMP_H
#define PUMP_H

class Pump : public Device {
public:
  Pump(int8_t pin, uint32_t traceInterval, uint32_t duration);
  void ArmToggle(bool armed);
  void Await();

  void SetEvent();
  void SetTraceInterval(uint32_t traceInterval);
  void SetDuration(uint32_t duration);
  
private:
  uint32_t traceInterval;
  uint32_t duration;
  uint32_t startTimestamp;
  uint32_t endTimestamp;

  void On();
  void Off();
};

#endif // PUMP_H
