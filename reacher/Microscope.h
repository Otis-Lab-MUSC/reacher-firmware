#ifndef MICROSCOPE_H
#define MICROSCOPE_H

#include <Arduino.h>

class Microscope {
public:
  Microscope(int8_t triggerPin, int8_t timestampPin);

  static void TimestampISR();
  void HandleFrameSignal();
  void ArmToggle(bool armed);
  void SetOffset(uint32_t offset);
  void Trigger();

  byte TriggerPin() const;
  byte TimestampPin() const;

private:
  int8_t triggerPin;
  int8_t timestampPin;
  volatile bool received;
  bool armed;
  volatile uint32_t timestamp;
  uint32_t offset;

  static Microscope* instance;

  void LogOutput();
};

#endif // MICROSCOPE_H
