#ifndef ACTION_H
#define ACTION_H

#include <Arduino.h>
#include "Device.h"

static constexpr uint8_t MAX_CHAIN_STEPS = 6;

enum class ActionType : uint8_t {
  ACTIVATE_DEVICE,   // Call device->Activate(startTs, param)
  SET_TIMEOUT,       // Set lever timeout end
  RESET_TRIGGER,     // Reset trigger press count
  NONE
};

struct Action {
  ActionType type;
  DeviceType target;
  uint32_t   offsetMs;   // Delay from trigger fire time (0 = immediate)
  uint32_t   param;      // Duration for ACTIVATE, timeout length for SET_TIMEOUT
};

struct Chain {
  Action   steps[MAX_CHAIN_STEPS];
  uint8_t  numSteps;
};

struct PendingAction {
  Action   action;
  uint32_t executeAt;    // Absolute timestamp to execute
  bool     active;
};

#endif // ACTION_H
