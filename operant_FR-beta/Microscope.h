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

private:
    int8_t triggerPin;
    int8_t timestampPin;
    bool received;
    bool armed;
    uint32_t timestamp;  
    uint32_t offset;

    static Microscope* instance;
};

#endif // MICROSCOPE_H
