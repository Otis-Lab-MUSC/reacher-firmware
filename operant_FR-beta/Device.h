#include <Arduino.h>

#ifndef DEVICE_H
#define DEVICE_H

class Device {
public:
  Device(int8_t pin, uint8_t mode);
  
  virtual void ArmToggle(bool arm);
  virtual void SetOffset(uint32_t offset);
  
  virtual byte Pin() const;
  virtual bool Armed() const; 
  virtual uint32_t Offset() const;
  virtual void Config(JsonDocument* doc);
  
private:
  uint32_t offset;
  const char deviceType;
  const char eventType;

  virtual void LogOutput();
  
protected:
  int8_t pin;
  uint8_t mode;
  bool armed;
  StaticJsonDocument<128> doc;
  
  void SerializeVar(const char var[], bool val);
  void SerializeVar(const char var[], uint32_t val);
};

#endif // DEVICE_H
