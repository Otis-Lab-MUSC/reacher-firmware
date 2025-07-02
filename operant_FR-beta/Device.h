#include <Arduino.h>

#ifndef DEVICE_H
#define DEVICE_H

class Device {
public:
  Device(int8_t pin, uint8_t mode, const char* device, const char* event);
  
  virtual void ArmToggle(bool arm);
  virtual void SetOffset(uint32_t offset);
  virtual void LogOutput();
  
  virtual byte Pin() const;
  virtual bool Armed() const; 
  virtual uint32_t Offset() const;
  
private:
  uint32_t offset;
  
protected:
  int8_t pin;
  uint8_t mode;
  bool armed;
  const char* device;
  const char* event;
  JsonDocument doc;
};

#endif // DEVICE_H
