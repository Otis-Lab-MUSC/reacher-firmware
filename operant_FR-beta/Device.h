#include <Arduino.h>

#ifndef DEVICE_H
#define DEVICE_H

class Device {
public:
  Device(int8_t pin, uint8_t mode);
  
  virtual void ArmToggle(bool arm);
  virtual void SetOffset(uint32_t offset);
  
  virtual int8_t Pin() const;
  virtual bool Armed() const; 
  virtual uint32_t Offset() const;
  
private:
  uint32_t offset;

  virtual void LogOutput();
  
protected:
  int8_t pin;
  uint8_t mode;
  bool armed;
};

#endif // DEVICE_H
