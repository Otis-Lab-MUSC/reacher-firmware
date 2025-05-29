#include <Arduino.h>

#ifndef DEVICE_H
#define DEVICE_H

class Device {
public:
  Device(int8_t pin, uint8_t mode);
  
  virtual void ArmToggle(bool arm);
  virtual void EventHandler();
  
  virtual int8_t Pin() const;
  virtual bool Armed() const; 
  
private:

protected:
  int8_t pin;
  uint8_t mode;
  bool armed;
};

#endif // DEVICE_H
