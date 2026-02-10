#ifndef CONFIG_H
#define CONFIG_H

#include "Scheduler.h"
#include "Cue.h"
#include "Pump.h"
#include "Laser.h"

// Default parameter values (matching operant_FR defaults)
static constexpr uint32_t DEFAULT_CUE_FREQUENCY      = 8000;
static constexpr uint32_t DEFAULT_CUE_DURATION       = 1600;
static constexpr uint32_t DEFAULT_PUMP_DURATION      = 2000;
static constexpr uint8_t  DEFAULT_LASER_FREQUENCY    = 40;
static constexpr uint32_t DEFAULT_LASER_DURATION     = 5000;
static constexpr uint32_t DEFAULT_TIMEOUT_INTERVAL   = 20000;

inline void configureFixedRatio(Scheduler& sched, Cue& cue, Pump& pump, Laser& laser, uint8_t ratio) {
  // Trigger 0: PRESS_COUNT for FR
  Trigger* t = sched.GetTrigger(0);
  if (t) {
    t->type = TriggerType::PRESS_COUNT;
    t->chainIndex = 0;
    t->enabled = true;
    t->threshold = ratio;
    t->pressCount = 0;
    t->prStep = 0;    // Fixed ratio (no progression)
  }

  // Disable trigger 1 (in case another paradigm configured it)
  Trigger* t1 = sched.GetTrigger(1);
  if (t1) t1->enabled = false;

  // Chain 0: Reward sequence
  //   Step 0: CUE immediately for cueDuration
  //   Step 1: PUMP after cueDuration for pumpDuration
  //   Step 2: LASER after cueDuration for laserDuration
  //   Step 3: SET_TIMEOUT on active lever immediately for timeoutInterval
  Chain* c = sched.GetChain(0);
  if (c) {
    c->numSteps = 4;

    // Cue fires immediately
    c->steps[0].type = ActionType::ACTIVATE_DEVICE;
    c->steps[0].target = DeviceType::CUE;
    c->steps[0].offsetMs = 0;
    c->steps[0].param = cue.Duration();

    // Pump fires after cue duration (trace interval)
    c->steps[1].type = ActionType::ACTIVATE_DEVICE;
    c->steps[1].target = DeviceType::PUMP;
    c->steps[1].offsetMs = cue.Duration();
    c->steps[1].param = pump.Duration();

    // Laser fires after cue duration
    c->steps[2].type = ActionType::ACTIVATE_DEVICE;
    c->steps[2].target = DeviceType::LASER;
    c->steps[2].offsetMs = cue.Duration();
    c->steps[2].param = laser.Duration();

    // Timeout starts immediately
    c->steps[3].type = ActionType::SET_TIMEOUT;
    c->steps[3].target = DeviceType::LEVER_RH;
    c->steps[3].offsetMs = 0;
    c->steps[3].param = sched.TimeoutInterval();
  }
}

inline void configureProgressiveRatio(Scheduler& sched, Cue& cue, Pump& pump, Laser& laser, uint8_t initialRatio, uint8_t step) {
  Trigger* t = sched.GetTrigger(0);
  if (t) {
    t->type = TriggerType::PRESS_COUNT;
    t->chainIndex = 0;
    t->enabled = true;
    t->threshold = initialRatio;
    t->pressCount = 0;
    t->prStep = step;
  }

  // Disable trigger 1 (in case another paradigm configured it)
  Trigger* t1 = sched.GetTrigger(1);
  if (t1) t1->enabled = false;

  Chain* c = sched.GetChain(0);
  if (c) {
    c->numSteps = 4;

    c->steps[0].type = ActionType::ACTIVATE_DEVICE;
    c->steps[0].target = DeviceType::CUE;
    c->steps[0].offsetMs = 0;
    c->steps[0].param = cue.Duration();

    c->steps[1].type = ActionType::ACTIVATE_DEVICE;
    c->steps[1].target = DeviceType::PUMP;
    c->steps[1].offsetMs = cue.Duration();
    c->steps[1].param = pump.Duration();

    c->steps[2].type = ActionType::ACTIVATE_DEVICE;
    c->steps[2].target = DeviceType::LASER;
    c->steps[2].offsetMs = cue.Duration();
    c->steps[2].param = laser.Duration();

    c->steps[3].type = ActionType::SET_TIMEOUT;
    c->steps[3].target = DeviceType::LEVER_RH;
    c->steps[3].offsetMs = 0;
    c->steps[3].param = sched.TimeoutInterval();
  }
}

inline void configureOmission(Scheduler& sched, Cue& cue, Pump& pump, Laser& laser, uint32_t absenceMs) {
  Trigger* t = sched.GetTrigger(0);
  if (t) {
    t->type = TriggerType::ABSENCE_TIMER;
    t->chainIndex = 0;
    t->enabled = true;
    t->absenceMs = absenceMs;
    t->absenceStart = 0;  // initialized in StartSession()
  }

  // Disable trigger 1
  Trigger* t1 = sched.GetTrigger(1);
  if (t1) t1->enabled = false;

  // Omission: cue, pump, laser all fire simultaneously (no trace interval, no timeout)
  Chain* c = sched.GetChain(0);
  if (c) {
    c->numSteps = 3;

    c->steps[0].type = ActionType::ACTIVATE_DEVICE;
    c->steps[0].target = DeviceType::CUE;
    c->steps[0].offsetMs = 0;
    c->steps[0].param = cue.Duration();

    c->steps[1].type = ActionType::ACTIVATE_DEVICE;
    c->steps[1].target = DeviceType::PUMP;
    c->steps[1].offsetMs = 0;
    c->steps[1].param = pump.Duration();

    c->steps[2].type = ActionType::ACTIVATE_DEVICE;
    c->steps[2].target = DeviceType::LASER;
    c->steps[2].offsetMs = 0;
    c->steps[2].param = laser.Duration();
  }
}

inline void configureVariableInterval(Scheduler& sched, Cue& cue, Pump& pump, Laser& laser, uint32_t totalInterval) {
  Trigger* t = sched.GetTrigger(0);
  if (t) {
    t->type = TriggerType::AVAILABILITY_WINDOW;
    t->chainIndex = 0;
    t->enabled = true;
    t->intervalMin = totalInterval;
    t->windowStart = 0;   // initialized in StartSession()
    t->windowEnd = 0;
    t->firedInWindow = false;
  }

  // Disable trigger 1
  Trigger* t1 = sched.GetTrigger(1);
  if (t1) t1->enabled = false;

  Chain* c = sched.GetChain(0);
  if (c) {
    c->numSteps = 4;

    c->steps[0].type = ActionType::ACTIVATE_DEVICE;
    c->steps[0].target = DeviceType::CUE;
    c->steps[0].offsetMs = 0;
    c->steps[0].param = cue.Duration();

    c->steps[1].type = ActionType::ACTIVATE_DEVICE;
    c->steps[1].target = DeviceType::PUMP;
    c->steps[1].offsetMs = cue.Duration();
    c->steps[1].param = pump.Duration();

    c->steps[2].type = ActionType::ACTIVATE_DEVICE;
    c->steps[2].target = DeviceType::LASER;
    c->steps[2].offsetMs = cue.Duration();
    c->steps[2].param = laser.Duration();

    c->steps[3].type = ActionType::SET_TIMEOUT;
    c->steps[3].target = DeviceType::LEVER_RH;
    c->steps[3].offsetMs = 0;
    c->steps[3].param = sched.TimeoutInterval();
  }
}

#endif // CONFIG_H
