#ifndef PUMP_H
#define PUMP_H

#include <Arduino.h>
#include "Device.h"

class Pump : public Device {
public:
  Pump(int8_t pin, uint32_t duration);

  void Activate(uint32_t startTs, uint32_t dur);
  void Await(uint32_t currentTimestamp);

  void SetDuration(uint32_t duration);
  uint32_t Duration() const;

private:
  uint32_t duration;
  uint32_t startTimestamp;
  uint32_t endTimestamp;

  void On();
  void Off();
};

#endif // PUMP_H
