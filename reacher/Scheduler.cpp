#include <ArduinoJson.h>
#include "Scheduler.h"
#include "SwitchLever.h"
#include "LickCircuit.h"
#include "Cue.h"
#include "Pump.h"
#include "Laser.h"
#include "Microscope.h"

Scheduler::Scheduler() {
  leverRH = nullptr;
  leverLH = nullptr;
  lickCircuit = nullptr;
  cue = nullptr;
  pump = nullptr;
  laser = nullptr;
  microscope = nullptr;
  sessionOffset = 0;
  timeoutInterval = 20000;
  sessionActive = false;
  lastPressClassRH = PressClass::INACTIVE;
  lastPressClassLH = PressClass::INACTIVE;

  for (uint8_t i = 0; i < MAX_TRIGGERS; i++) {
    triggers[i].enabled = false;
    triggers[i].pressCount = 0;
  }
  for (uint8_t i = 0; i < MAX_CHAINS; i++) {
    chains[i].numSteps = 0;
  }
  for (uint8_t i = 0; i < MAX_PENDING; i++) {
    pending[i].active = false;
  }
}

void Scheduler::RegisterLever(SwitchLever* lever, DeviceType type) {
  if (type == DeviceType::LEVER_RH) leverRH = lever;
  else if (type == DeviceType::LEVER_LH) leverLH = lever;
}

void Scheduler::RegisterLickCircuit(LickCircuit* lick) {
  lickCircuit = lick;
}

void Scheduler::RegisterCue(Cue* c) {
  cue = c;
}

void Scheduler::RegisterPump(Pump* p) {
  pump = p;
}

void Scheduler::RegisterLaser(Laser* l) {
  laser = l;
}

void Scheduler::RegisterMicroscope(Microscope* mic) {
  microscope = mic;
}

void Scheduler::Update(uint32_t now) {
  // Tick time-based triggers
  for (uint8_t i = 0; i < MAX_TRIGGERS; i++) {
    if (triggers[i].OnTick(now)) {
      FireChain(triggers[i].chainIndex, now);
    }
  }

  // Execute pending actions whose time has come
  TickPending(now);

  // Tick output device state machines
  TickOutputs(now);
}

void Scheduler::OnInputEvent(DeviceType source, uint32_t timestamp) {
  if (!sessionActive) return;

  // Only handle lever events for triggering
  if (source != DeviceType::LEVER_RH && source != DeviceType::LEVER_LH) return;

  PressClass cls = ClassifyPress(source, timestamp);

  // Store classification for logging on release
  if (source == DeviceType::LEVER_RH) {
    lastPressClassRH = cls;
  } else {
    lastPressClassLH = cls;
  }

  // Every ACTIVE press resets the timeout window (matching original behavior)
  if (cls == PressClass::ACTIVE) {
    SwitchLever* lever = GetLever(source);
    if (lever) {
      lever->SetTimeoutEnd(timestamp + timeoutInterval);
    }

    // Offer to triggers; fire chain if threshold met
    for (uint8_t i = 0; i < MAX_TRIGGERS; i++) {
      if (triggers[i].OnInputEvent(source, timestamp)) {
        FireChain(triggers[i].chainIndex, timestamp);
      }
    }
  }
}

void Scheduler::OnInputRelease(DeviceType source) {
  if (!sessionActive) return;

  if (source == DeviceType::LEVER_RH) {
    LogLeverPress(source, lastPressClassRH);
  } else if (source == DeviceType::LEVER_LH) {
    LogLeverPress(source, lastPressClassLH);
  }
}

PressClass Scheduler::ClassifyPress(DeviceType source, uint32_t timestamp) {
  SwitchLever* lever = GetLever(source);
  if (!lever) return PressClass::INACTIVE;

  if (!lever->IsReinforced()) {
    return PressClass::INACTIVE;
  }

  if (lever->InTimeout(timestamp)) {
    return PressClass::TIMEOUT;
  }

  return PressClass::ACTIVE;
}

void Scheduler::FireChain(uint8_t chainIndex, uint32_t now) {
  if (chainIndex >= MAX_CHAINS) return;
  const Chain& chain = chains[chainIndex];

  for (uint8_t i = 0; i < chain.numSteps; i++) {
    const Action& action = chain.steps[i];
    if (action.type == ActionType::NONE) continue;

    if (action.offsetMs == 0) {
      ExecuteAction(action, now);
    } else {
      SchedulePending(action, now + action.offsetMs);
    }
  }
}

void Scheduler::ExecuteAction(const Action& action, uint32_t now) {
  switch (action.type) {
    case ActionType::ACTIVATE_DEVICE:
      switch (action.target) {
        case DeviceType::CUE:
          if (cue && cue->Armed()) {
            cue->Activate(now, action.param);
            LogDeviceActivation(DeviceType::CUE, now, now + action.param);
          }
          break;
        case DeviceType::PUMP:
          if (pump && pump->Armed()) {
            pump->Activate(now, action.param);
            LogDeviceActivation(DeviceType::PUMP, now, now + action.param);
          }
          break;
        case DeviceType::LASER:
          if (laser && laser->Armed() && laser->IsContingent()) {
            laser->Activate(now, action.param);
            LogDeviceActivation(DeviceType::LASER, now, now + action.param);
          }
          break;
        default:
          break;
      }
      break;

    case ActionType::SET_TIMEOUT: {
      SwitchLever* lever = GetLever(action.target);
      if (lever) {
        lever->SetTimeoutEnd(now + action.param);
      }
      break;
    }

    case ActionType::RESET_TRIGGER:
      for (uint8_t i = 0; i < MAX_TRIGGERS; i++) {
        triggers[i].Reset();
      }
      break;

    default:
      break;
  }
}

void Scheduler::TickPending(uint32_t now) {
  for (uint8_t i = 0; i < MAX_PENDING; i++) {
    if (pending[i].active && now >= pending[i].executeAt) {
      pending[i].active = false;
      ExecuteAction(pending[i].action, now);
    }
  }
}

void Scheduler::TickOutputs(uint32_t now) {
  if (cue) cue->Await(now);
  if (pump) pump->Await(now);
  if (laser) laser->Await(now);
}

void Scheduler::SchedulePending(const Action& action, uint32_t fireTime) {
  for (uint8_t i = 0; i < MAX_PENDING; i++) {
    if (!pending[i].active) {
      pending[i].action = action;
      pending[i].executeAt = fireTime;
      pending[i].active = true;
      return;
    }
  }
}

void Scheduler::LogLeverPress(DeviceType source, PressClass cls) {
  SwitchLever* lever = GetLever(source);
  if (!lever) return;

  JsonDocument doc;
  doc[F("level")] = F("007");
  doc[F("device")] = F("SWITCH_LEVER");
  doc[F("pin")] = lever->Pin();
  doc[F("event")] = F("PRESS");

  switch (cls) {
    case PressClass::ACTIVE:   doc[F("class")] = F("ACTIVE");   break;
    case PressClass::INACTIVE: doc[F("class")] = F("INACTIVE"); break;
    case PressClass::TIMEOUT:  doc[F("class")] = F("TIMEOUT");  break;
  }

  doc[F("start_timestamp")] = lever->StartTimestamp() - sessionOffset;
  doc[F("end_timestamp")] = lever->EndTimestamp() - sessionOffset;
  doc[F("orientation")] = lever->Orientation();

  serializeJson(doc, Serial);
  Serial.println();
}

void Scheduler::LogDeviceActivation(DeviceType target, uint32_t startTs, uint32_t endTs) {
  JsonDocument doc;
  doc[F("level")] = F("007");

  switch (target) {
    case DeviceType::CUE:
      doc[F("device")] = F("CUE");
      doc[F("pin")] = cue->Pin();
      doc[F("event")] = F("TONE");
      break;
    case DeviceType::PUMP:
      doc[F("device")] = F("PUMP");
      doc[F("pin")] = pump->Pin();
      doc[F("event")] = F("INFUSION");
      break;
    case DeviceType::LASER:
      doc[F("device")] = F("LASER");
      doc[F("pin")] = laser->Pin();
      doc[F("event")] = F("STIM");
      break;
    default:
      return;
  }

  doc[F("start_timestamp")] = startTs - sessionOffset;
  doc[F("end_timestamp")] = endTs - sessionOffset;

  serializeJson(doc, Serial);
  Serial.println();
}

void Scheduler::SetTimeoutInterval(uint32_t interval) {
  timeoutInterval = interval;
  // Update the SET_TIMEOUT action in chain 0 if it exists
  Chain* c = GetChain(0);
  if (c) {
    for (uint8_t i = 0; i < c->numSteps; i++) {
      if (c->steps[i].type == ActionType::SET_TIMEOUT) {
        c->steps[i].param = interval;
      }
    }
  }
}

void Scheduler::SetRatio(uint8_t ratio) {
  for (uint8_t i = 0; i < MAX_TRIGGERS; i++) {
    if (triggers[i].type == TriggerType::PRESS_COUNT) {
      triggers[i].threshold = ratio;
      triggers[i].pressCount = 0;
      return;
    }
  }
}

uint32_t Scheduler::TimeoutInterval() const {
  return timeoutInterval;
}

uint32_t Scheduler::SessionOffset() const {
  return sessionOffset;
}

Trigger* Scheduler::GetTrigger(uint8_t index) {
  if (index < MAX_TRIGGERS) return &triggers[index];
  return nullptr;
}

Chain* Scheduler::GetChain(uint8_t index) {
  if (index < MAX_CHAINS) return &chains[index];
  return nullptr;
}

void Scheduler::StartSession(uint32_t now) {
  sessionOffset = now;
  sessionActive = true;

  // Reset all triggers
  for (uint8_t i = 0; i < MAX_TRIGGERS; i++) {
    triggers[i].Reset();
  }

  // Initialize time-based triggers
  for (uint8_t i = 0; i < MAX_TRIGGERS; i++) {
    if (!triggers[i].enabled) continue;
    if (triggers[i].type == TriggerType::ABSENCE_TIMER) {
      triggers[i].absenceStart = now;
    } else if (triggers[i].type == TriggerType::AVAILABILITY_WINDOW) {
      triggers[i].windowStart = now + random(0, triggers[i].intervalMin);
      triggers[i].windowEnd = now + triggers[i].intervalMin;
      triggers[i].firedInWindow = false;
    }
  }

  // Clear pending queue
  for (uint8_t i = 0; i < MAX_PENDING; i++) {
    pending[i].active = false;
  }

  // Reset lever timeouts
  if (leverRH) leverRH->SetTimeoutEnd(0);
  if (leverLH) leverLH->SetTimeoutEnd(0);
}

void Scheduler::EndSession(uint32_t now) {
  sessionActive = false;

  // Clear pending queue
  for (uint8_t i = 0; i < MAX_PENDING; i++) {
    pending[i].active = false;
  }

  // Force all outputs off
  if (cue) noTone(cue->Pin());
  if (pump) digitalWrite(pump->Pin(), LOW);
  if (laser) digitalWrite(laser->Pin(), LOW);
}

SwitchLever* Scheduler::GetLever(DeviceType type) {
  if (type == DeviceType::LEVER_RH) return leverRH;
  if (type == DeviceType::LEVER_LH) return leverLH;
  return nullptr;
}
