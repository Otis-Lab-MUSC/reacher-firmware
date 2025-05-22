#include <Arduino.h>

#ifndef DEVICE_H
#define DEVICE_H

class Device {
  protected:
    int8_t pin;
    uint8_t mode;
    bool armed;
  public:
    Device(int8_t _pin, uint8_t _mode);
    virtual void ArmToggle();
    virtual int8_t Pin() const;
    virtual bool Armed() const; 
};

#endif // DEVICE_H
