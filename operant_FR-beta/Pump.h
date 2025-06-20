#include <Arduino.h>
#include "Device.h"

#ifndef PUMP_H
#define PUMP_H

class Pump : public Device {
public:
  Pump(int8_t pin, uint32_t duration, uint32_t traceInterval);
  void ArmToggle(bool armed);
  void Await(uint32_t currentTimestamp);

  void SetEvent(uint32_t currentTimestamp);
  void SetDuration(uint32_t duration);
  void SetTraceInterval(uint32_t traceInterval);
  void Config(JsonDocument* doc);

  uint32_t Duration();
  uint32_t TraceInterval();
  
private:
  uint32_t duration;
  uint32_t traceInterval;
  uint32_t startTimestamp;
  uint32_t endTimestamp;
  JsonDocument doc;
  const char deviceType[];
  const char eventType[];

  void On();
  void Off();
  void LogOutput();
};

#endif // PUMP_H
