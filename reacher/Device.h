#ifndef DEVICE_H
#define DEVICE_H

#include <Arduino.h>

enum class DeviceType : uint8_t {
  LEVER_RH,
  LEVER_LH,
  LICK,
  CUE,
  PUMP,
  LASER,
  MICROSCOPE,
  NONE
};

class Device {
public:
  Device(int8_t pin, uint8_t mode, const char* device);

  void ArmToggle(bool arm);
  void SetOffset(uint32_t offset);

  byte Pin() const;
  bool Armed() const;
  uint32_t Offset() const;

private:
  uint32_t offset;

protected:
  int8_t pin;
  uint8_t mode;
  bool armed;
  const char* device;
};

// Callback typedefs shared by input devices
typedef void (*InputEventCallback)(DeviceType source, uint32_t timestamp);
typedef void (*InputReleaseCallback)(DeviceType source);

#endif // DEVICE_H
