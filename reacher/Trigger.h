#ifndef TRIGGER_H
#define TRIGGER_H

#include <Arduino.h>
#include "Device.h"

enum class TriggerType : uint8_t {
  PRESS_COUNT,           // FR/PR: N active presses -> fire chain
  ABSENCE_TIMER,         // Omission: no press for N ms -> fire chain
  AVAILABILITY_WINDOW,   // VI: random window, press during it -> fire chain
  MANUAL                 // Serial command test trigger
};

enum class PressClass : uint8_t {
  ACTIVE,
  INACTIVE,
  TIMEOUT
};

struct Trigger {
  TriggerType type       = TriggerType::PRESS_COUNT;
  uint8_t     chainIndex = 0;
  bool        enabled    = false;

  // PRESS_COUNT fields
  uint8_t     threshold  = 1;
  uint8_t     pressCount = 0;
  uint8_t     prStep     = 0;

  // ABSENCE_TIMER fields
  uint32_t    absenceMs    = 0;
  uint32_t    absenceStart = 0;

  // AVAILABILITY_WINDOW fields
  uint32_t    windowStart  = 0;
  uint32_t    windowEnd    = 0;
  uint32_t    intervalMin  = 0;
  bool        firedInWindow = false;

  // Returns true if trigger should fire
  bool OnInputEvent(DeviceType source, uint32_t timestamp) {
    if (!enabled) return false;

    switch (type) {
      case TriggerType::PRESS_COUNT:
        pressCount++;
        if (pressCount >= threshold) {
          pressCount = 0;
          if (prStep > 0) {
            threshold += prStep;
          }
          return true;
        }
        return false;

      case TriggerType::ABSENCE_TIMER:
        absenceStart = timestamp;
        return false;

      case TriggerType::AVAILABILITY_WINDOW:
        if (!firedInWindow && timestamp >= windowStart && timestamp < windowEnd) {
          firedInWindow = true;
          return true;
        }
        return false;

      default:
        return false;
    }
  }

  // Called every loop tick; returns true if trigger should fire
  bool OnTick(uint32_t now) {
    if (!enabled) return false;

    switch (type) {
      case TriggerType::ABSENCE_TIMER:
        if (absenceStart > 0 && (now - absenceStart) >= absenceMs) {
          absenceStart = now;  // restart timer immediately for next omission cycle
          return true;
        }
        return false;

      case TriggerType::AVAILABILITY_WINDOW:
        if (windowEnd > 0 && now >= windowEnd) {
          // Interval expired, start new interval with new random window
          windowStart = now + random(0, intervalMin);
          windowEnd = now + intervalMin;
          firedInWindow = false;
        }
        return false;

      default:
        return false;
    }
  }

  void Reset() {
    pressCount = 0;
    absenceStart = 0;
    windowStart = 0;
    windowEnd = 0;
    firedInWindow = false;
    if (prStep > 0) {
      threshold = prStep; // Reset to initial PR step
    }
  }
};

#endif // TRIGGER_H
