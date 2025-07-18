#include <Arduino.h>
#include "Device.h"
#include "Cue.h"
#include "Pump.h"
#include "Laser.h"

#ifndef SWITCHLEVER_H
#define SWITCHLEVER_H

class SwitchLever : public Device {
public:
  SwitchLever(int8_t pin, const char* orientation);
  void Monitor(uint32_t currentTimestamp);

  void SetCue(Cue* cue);
  void SetPump(Pump* cue);
  void SetLaser(Laser* laser);
  void SetTimeoutIntervalLength(uint32_t timeoutInterval);
  void SetActiveLever(bool reinforced);
  void SetRatio(uint8_t ratio);

  JsonDocument Defaults();
  
private:
  bool initState;
  bool previousState;
  bool stableState;
  char orientation[3];
  bool reinforced;
  uint32_t timeoutInterval;
  uint32_t timeoutIntervalEnd;
  uint32_t lastDebounceTimestamp;
  uint8_t debounceDelay;
  uint32_t startTimestamp;
  uint32_t endTimestamp;
  enum PressType { INACTIVE, ACTIVE, TIMEOUT };
  PressType pressType;
  uint8_t ratio;
  uint8_t numPresses;
  Cue* cue;
  Pump* pump;
  Laser* laser;

  void Classify(uint32_t pressTimestamp, uint32_t currentTimestamp);
  void LogOutput();
  void AddActions(uint32_t currentTimestamp);
};

#endif // SWITCHLEVER_H
