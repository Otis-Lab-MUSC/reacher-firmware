#ifndef LASER_H
#define LASER_H

#include <Arduino.h>
#include "Device.h"

class Laser : public Device {
public:
  Laser(int8_t pin, uint32_t frequency, uint32_t duration);

  void Activate(uint32_t startTs, uint32_t dur);
  void Await(uint32_t currentTimestamp);
  void Test(uint32_t currentTimestamp);

  void SetFrequency(uint32_t frequency);
  void SetDuration(uint32_t duration);
  void SetMode(bool contingent);

  uint32_t Frequency() const;
  uint32_t Duration() const;
  bool IsContingent() const;

private:
  uint32_t frequency;
  uint32_t duration;
  uint32_t startTimestamp;
  uint32_t endTimestamp;
  uint32_t halfCycleStartTimestamp;
  uint32_t halfCycleEndTimestamp;
  enum Mode : uint8_t { CONTINGENT, INDEPENDENT };
  Mode mode;
  bool state;
  bool halfState;
  bool isTesting;

  void On();
  void Off();
  void Cycle(uint32_t currentTimestamp);
  void Oscillate(uint32_t currentTimestamp);
  void UpdateHalfCycle(uint32_t currentTimestamp);
};

#endif // LASER_H
