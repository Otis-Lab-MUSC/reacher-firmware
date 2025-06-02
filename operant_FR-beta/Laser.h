#include <Arduino.h>
#include "Device.h"

#ifndef LASER_H
#define LASER_H

class Laser : public Device {
public:
  Laser(int8_t pin);
  void ArmToggle(bool armed);
  void Await();

  void SetEvent();
  void SetDuration(uint32_t duration);
  void SetFrequency(uint32_t frequency);

  uint32_t Duration();
  uint32_t Frequency();
  
private:

};

#endif // LASER_H
