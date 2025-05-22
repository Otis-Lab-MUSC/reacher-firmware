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
    SwitchLever(byte pin, int8_t mode);
    void ArmToggle();
    void SetPreviousState(bool _previousState);
    void SetStableState(bool _stableState);
    void SetOrientation(char _orientation[2]);

    bool PreviousState() const;
    bool StableState() const;
    char Orientation() const;
  protected:
};

#endif // SWITCHLEVER_H
