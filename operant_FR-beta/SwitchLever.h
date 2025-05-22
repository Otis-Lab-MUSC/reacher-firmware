#include <Arduino.h>
#include "Device.h"

#ifndef SWITCHLEVER_H
#define SWITCHLEVER_H

class SwitchLever : public Device {
  private:
    char orientation[2];
    bool previousState;
    bool stableState;
  public:
    SwitchLever(int8_t _pin, int8_t _mode);
    void ArmToggle();
    void SetPreviousState(bool _previousState);
    void SetStableState(bool _stableState);
    void SetOrientation(char _orientation[2]);
    void Monitor();

    bool PreviousState() const;
    bool StableState() const;
    const char* Orientation() const;
  protected:
};

class Press {
  private:
    uint32_t pressTimestamp;
    uint32_t releaseTimestamp;
    enum class PressType {
      ACTIVE,
      INACTIVE,
      TIMEOUT,
      INDEPENDENT
    };
    PressType pressType; 
  public:
    void DefinePressType();
    void SendOutput();
  protected:
};

#endif // SWITCHLEVER_H
