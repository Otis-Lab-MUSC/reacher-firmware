#ifndef SWITCHLEVER_H
#define SWITCHLEVER_H

#include <Arduino.h>
#include "Device.h"

class SwitchLever : public Device {
public:
  SwitchLever(int8_t pin, const char* orientation, DeviceType type);

  void Monitor(uint32_t currentTimestamp);
  void SetCallback(InputEventCallback pressCb);
  void SetReleaseCallback(InputReleaseCallback releaseCb);
  void SetActiveLever(bool reinforced);
  void SetTimeoutEnd(uint32_t endTs);

  bool IsReinforced() const;
  bool InTimeout(uint32_t ts) const;
  const char* Orientation() const;
  DeviceType Type() const;
  uint32_t StartTimestamp() const;
  uint32_t EndTimestamp() const;

private:
  DeviceType devType;
  bool initState;
  bool previousState;
  bool stableState;
  char orientation[3];
  bool reinforced;
  uint32_t timeoutEnd;
  uint32_t lastDebounceTimestamp;
  uint8_t debounceDelay;
  uint32_t startTimestamp;
  uint32_t endTimestamp;
  InputEventCallback callback;
  InputReleaseCallback releaseCallback;
};

#endif // SWITCHLEVER_H
