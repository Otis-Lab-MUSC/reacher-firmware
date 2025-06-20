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
  void ArmToggle(bool armed);
  void Monitor(uint32_t currentTimestamp);

  void SetCue(Cue* cue);
  void SetPump(Pump* cue);
  void SetLaser(Laser* laser);
  void SetTimeoutIntervalLength(uint32_t timeoutInterval);
  void SetReinforcement(bool reinforced);
  void Config(JsonDocument* doc);
  
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
  enum PressType { ACTIVE, TIMEOUT, INDEPENDENT };
  PressType pressType;
  Cue* cue;
  Pump* pump;
  Laser* laser;
  JsonDocument doc;
  const char deviceType[];
  const char eventType[];

  void Classify(uint32_t pressTimestamp, uint32_t currentTimestamp);
  void LogOutput();
  void AddActions(uint32_t currentTimestamp);
};

#endif // SWITCHLEVER_H
