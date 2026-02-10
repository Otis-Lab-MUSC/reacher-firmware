#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>
#include "Device.h"
#include "Trigger.h"
#include "Action.h"

class SwitchLever;
class LickCircuit;
class Cue;
class Pump;
class Laser;
class Microscope;

static constexpr uint8_t MAX_TRIGGERS = 2;
static constexpr uint8_t MAX_CHAINS   = 2;
static constexpr uint8_t MAX_PENDING  = 8;

class Scheduler {
public:
  Scheduler();

  // Device registration
  void RegisterLever(SwitchLever* lever, DeviceType type);
  void RegisterLickCircuit(LickCircuit* lick);
  void RegisterCue(Cue* cue);
  void RegisterPump(Pump* pump);
  void RegisterLaser(Laser* laser);
  void RegisterMicroscope(Microscope* mic);

  // Main loop entry point
  void Update(uint32_t now);

  // Called by input device callbacks
  void OnInputEvent(DeviceType source, uint32_t timestamp);
  void OnInputRelease(DeviceType source);

  // Trigger / chain configuration
  Trigger* GetTrigger(uint8_t index);
  Chain*   GetChain(uint8_t index);
  void     SetTimeoutInterval(uint32_t interval);
  void     SetRatio(uint8_t ratio);

  // Session management
  void StartSession(uint32_t now);
  void EndSession(uint32_t now);

  uint32_t TimeoutInterval() const;
  uint32_t SessionOffset() const;

private:
  // Device pointers
  SwitchLever* leverRH;
  SwitchLever* leverLH;
  LickCircuit* lickCircuit;
  Cue*         cue;
  Pump*        pump;
  Laser*       laser;
  Microscope*  microscope;

  // Triggers and chains
  Trigger triggers[MAX_TRIGGERS];
  Chain   chains[MAX_CHAINS];

  // Pending action queue
  PendingAction pending[MAX_PENDING];

  // Session state
  uint32_t sessionOffset;
  uint32_t timeoutInterval;
  bool     sessionActive;

  // Per-lever press classification (stored on press, logged on release)
  PressClass lastPressClassRH;
  PressClass lastPressClassLH;

  // Internal methods
  PressClass ClassifyPress(DeviceType source, uint32_t timestamp);
  void FireChain(uint8_t chainIndex, uint32_t now);
  void ExecuteAction(const Action& action, uint32_t now);
  void TickPending(uint32_t now);
  void TickOutputs(uint32_t now);
  void LogLeverPress(DeviceType source, PressClass cls);
  void LogDeviceActivation(DeviceType target, uint32_t startTs, uint32_t endTs);
  void SchedulePending(const Action& action, uint32_t fireTime);

  SwitchLever* GetLever(DeviceType type);
};

#endif // SCHEDULER_H
