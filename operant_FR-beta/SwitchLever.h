#include <Arduino.h>
#include "Device.h"

#ifndef SWITCHLEVER_H
#define SWITCHLEVER_H

class SwitchLever : public Device {
public:
  SwitchLever(int8_t _pin, const char* _orientation, bool _reinforced);
  void ArmToggle();
  void Monitor();
  
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
  uint32_t pressTimestamp;
  uint32_t releaseTimestamp;
  enum class PressType {
    ACTIVE,
    TIMEOUT,
    INDEPENDENT
  };
  PressType pressType;
  
protected:
};

#endif // SWITCHLEVER_H
