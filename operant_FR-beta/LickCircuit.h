#include <Arduino.h>
#include "Device.h"

#ifndef LICKCIRCUIT_H
#define LICKCIRCUIT_H

class LickCircuit : public Device {
public:
  LickCircuit(int8_t pin);
  void ArmToggle(bool armed);
  void Monitor(uint32_t currentTimestamp);
  
private:
  bool initState;
  bool previousState;
  bool stableState;
  uint32_t lastDebounceTimestamp;
  uint8_t debounceDelay;
  uint32_t startTimestamp;
  uint32_t endTimestamp;

  void LogOutput();
};

#endif // LICKCIRCUIT_H
