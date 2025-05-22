#include <Arduino.h>

#ifndef DEVICE_H
#define DEVICE_H

class Device {
  protected:
    byte pin;
    uint8_t mode;
    bool armed;
  public:
    Device(byte pin, uint8_t mode);
    virtual void ArmToggle();
    virtual byte Pin() const;
    virtual bool Armed() const; 
};

#endif // DEVICE_H
