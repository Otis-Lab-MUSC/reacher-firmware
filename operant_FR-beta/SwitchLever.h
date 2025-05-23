#include <Arduino.h>
#include "Device.h"

#ifndef SWITCHLEVER_H
#define SWITCHLEVER_H

class SwitchLever : public Device {
public:
  SwitchLever(int8_t _pin, int8_t _mode, const char* _orientation);
  void ArmToggle();
  void SetActiveLever();
  void SetPreviousState(bool _previousState);
  void SetStableState(bool _stableState);
  void Monitor();
  
private:
  bool previousState;
  bool stableState;
  char orientation[3];
  uint32_t timeoutInterval;
  uint32_t timeoutIntervalEnd;
  uint32_t lastDebounceTimestamp;
  uint8_t debounceDelay;
  uint32_t pressTimestamp;
  uint32_t releaseTimestamp;
  enum class PressType {
    ACTIVE,
    INACTIVE,
    TIMEOUT,
    INDEPENDENT
  };
  PressType ClassifyPress(uint32_t _pressTimestamp);
  
protected:
};

#endif // SWITCHLEVER_H
