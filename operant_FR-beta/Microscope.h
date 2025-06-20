#include <Arduino.h>
#include "Device.h"

#ifndef MICROSCOPE_H
#define MICROSCOPE_H

class Microscope {
public:
    Microscope(int8_t triggerPin, int8_t timestampPin);
    
    static void TimestampISR();
    void HandleFrameSignal();
    void SetCollectFrames(bool state);
    void ArmToggle(bool armed);
    void SetOffset(uint32_t offset);
    void Config(JsonDocument* doc);

    byte TriggerPin();
    byte TimestampPin();

private:
  int8_t triggerPin;
  int8_t timestampPin;
  bool received;
  bool armed;
  uint32_t timestamp;  
  uint32_t offset;
  JsonDocument doc;
  const char deviceType[];
  const char eventType[];

    static Microscope* instance;

    void LogOutput();
};

#endif // MICROSCOPE_H
