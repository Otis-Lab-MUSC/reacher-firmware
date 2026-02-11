/**
 * @file PavlovianScheduler.cpp
 * @brief PavlovianScheduler implementation — trial state machine, ITI sampling, JSON logging.
 */

#include "PavlovianScheduler.h"
#include <SwitchLever.h>
#include <LickCircuit.h>
#include <Cue.h>
#include <Pump.h>
#include <Microscope.h>

PavlovianScheduler::PavlovianScheduler() {
  leverRH = nullptr;
  leverLH = nullptr;
  lickCircuit = nullptr;
  cue = nullptr;
  cue2 = nullptr;
  pump = nullptr;
  pump2 = nullptr;
  microscope = nullptr;
  sessionOffset = 0;
  sessionActive = false;
  lastPressClassRH = PressClass::INACTIVE;
  lastPressClassLH = PressClass::INACTIVE;

  pavCsPlusCount = 50;
  pavCsMinusCount = 50;
  pavCsPlusProb = 100;
  pavCsMinusProb = 0;
  pavCounterbalance = false;
  pavCueDuration = 2000;
  pavTraceInterval = 1000;
  pavConsumptionMs = 3000;
  pavItiMean = 30000;
  pavItiMin = 10000;
  pavItiMax = 90000;

  pavPhase = PavlovPhase::IDLE;
  pavTrialIndex = 0;
  pavTotalTrials = 0;
  pavRewardThisTrial = false;
  pavPhaseStart = 0;
  pavCurrentIti = 0;
  memset(pavTrialTypes, 0, TRIAL_ARRAY_BYTES);
}

void PavlovianScheduler::RegisterLever(SwitchLever* lever, DeviceType type) {
  if (type == DeviceType::LEVER_RH) leverRH = lever;
  else if (type == DeviceType::LEVER_LH) leverLH = lever;
}

void PavlovianScheduler::RegisterLickCircuit(LickCircuit* lick) {
  lickCircuit = lick;
}

void PavlovianScheduler::RegisterCue(Cue* c) {
  cue = c;
}

void PavlovianScheduler::RegisterCue2(Cue* c) {
  cue2 = c;
}

void PavlovianScheduler::RegisterPump(Pump* p) {
  pump = p;
}

void PavlovianScheduler::RegisterPump2(Pump* p) {
  pump2 = p;
}

void PavlovianScheduler::RegisterMicroscope(Microscope* mic) {
  microscope = mic;
}

void PavlovianScheduler::Update(uint32_t now) {
  if (sessionActive) PavTick(now);
  TickOutputs(now);
}

void PavlovianScheduler::OnInputEvent(DeviceType source, uint32_t timestamp) {
  if (!sessionActive) return;
  if (source != DeviceType::LEVER_RH && source != DeviceType::LEVER_LH) return;

  PressClass cls = ClassifyPress(source);

  if (source == DeviceType::LEVER_RH) {
    lastPressClassRH = cls;
  } else {
    lastPressClassLH = cls;
  }
  // Presses are logged but don't affect trial progression
}

void PavlovianScheduler::OnInputRelease(DeviceType source) {
  if (!sessionActive) return;

  if (source == DeviceType::LEVER_RH) {
    LogLeverPress(source, lastPressClassRH);
  } else if (source == DeviceType::LEVER_LH) {
    LogLeverPress(source, lastPressClassLH);
  }
}

PressClass PavlovianScheduler::ClassifyPress(DeviceType source) {
  SwitchLever* lever = GetLever(source);
  if (!lever) return PressClass::INACTIVE;
  // In Pavlovian mode, both levers are reinforced and there's no timeout
  return PressClass::ACTIVE;
}

void PavlovianScheduler::Configure(uint8_t csPlusCount, uint8_t csMinusCount,
                                   uint32_t csPlusFreq, uint32_t csMinusFreq,
                                   uint16_t cueDuration, uint16_t traceInterval, uint16_t consumptionMs,
                                   uint32_t itiMean, uint32_t itiMin, uint32_t itiMax,
                                   uint8_t csPlusProb, uint8_t csMinusProb, bool counterbalance) {
  // Reject mid-session reconfiguration to prevent trial order corruption
  if (sessionActive) return;

  pavCsPlusCount = csPlusCount;
  pavCsMinusCount = csMinusCount;
  pavCueDuration = cueDuration;
  pavTraceInterval = traceInterval;
  pavConsumptionMs = consumptionMs;
  pavItiMean = itiMean;
  pavItiMin = itiMin;
  pavItiMax = itiMax;
  pavCsPlusProb = csPlusProb;
  pavCsMinusProb = csMinusProb;
  pavCounterbalance = counterbalance;

  // Configure CS+ cue: continuous tone
  if (cue) {
    cue->SetFrequency(csPlusFreq);
    cue->SetPulsed(false);
  }
  // Configure CS- cue: pulsed tone
  if (cue2) {
    cue2->SetFrequency(csMinusFreq);
    cue2->SetPulsed(true, 200, 200);
  }
}

void PavlovianScheduler::StartSession(uint32_t now) {
  sessionOffset = now;
  sessionActive = true;

  pavTotalTrials = pavCsPlusCount + pavCsMinusCount;
  if (pavTotalTrials > MAX_PAVLOV_TRIALS) pavTotalTrials = MAX_PAVLOV_TRIALS;
  randomSeed(analogRead(A0) ^ now);
  PavGenerateTrialOrder();
  pavTrialIndex = 0;
  pavCurrentIti = PavSampleIti();
  pavPhase = PavlovPhase::ITI;
  pavPhaseStart = now;
}

void PavlovianScheduler::EndSession(uint32_t now) {
  sessionActive = false;
  pavPhase = PavlovPhase::IDLE;

  // Force all outputs off
  if (cue) noTone(cue->Pin());
  if (cue2) noTone(cue2->Pin());
  if (pump) digitalWrite(pump->Pin(), LOW);
  if (pump2) digitalWrite(pump2->Pin(), LOW);
}

uint32_t PavlovianScheduler::SessionOffset() const {
  return sessionOffset;
}

SwitchLever* PavlovianScheduler::GetLever(DeviceType type) {
  if (type == DeviceType::LEVER_RH) return leverRH;
  if (type == DeviceType::LEVER_LH) return leverLH;
  return nullptr;
}

void PavlovianScheduler::TickOutputs(uint32_t now) {
  if (cue) cue->Await(now);
  if (cue2) cue2->Await(now);
  if (pump) pump->Await(now);
  if (pump2) pump2->Await(now);
}

// --- Trial generation ---

bool PavlovianScheduler::PavIsCSMinus(uint8_t index) const {
  return (pavTrialTypes[index / 8] >> (index % 8)) & 1;
}

void PavlovianScheduler::PavSetTrialType(uint8_t index, bool isCsMinus) {
  uint8_t byteIdx = index / 8;
  uint8_t bitIdx = index % 8;
  if (isCsMinus) {
    pavTrialTypes[byteIdx] |= (1 << bitIdx);
  } else {
    pavTrialTypes[byteIdx] &= ~(1 << bitIdx);
  }
}

void PavlovianScheduler::PavGenerateTrialOrder() {
  uint8_t total = pavCsPlusCount + pavCsMinusCount;
  uint8_t csPlus = pavCsPlusCount;
  uint8_t csMinus = pavCsMinusCount;

  // Proportionally scale both counts if total exceeds MAX_PAVLOV_TRIALS
  if (total > MAX_PAVLOV_TRIALS) {
    csPlus = (uint8_t)((uint16_t)pavCsPlusCount * MAX_PAVLOV_TRIALS / total);
    csMinus = MAX_PAVLOV_TRIALS - csPlus;
    total = MAX_PAVLOV_TRIALS;
  }

  for (uint8_t attempt = 0; attempt < 50; attempt++) {
    // Fill array: CS+ first, CS- second
    memset(pavTrialTypes, 0, TRIAL_ARRAY_BYTES);
    for (uint8_t i = csPlus; i < total; i++) {
      PavSetTrialType(i, true);
    }

    // Fisher-Yates shuffle
    for (uint8_t i = total - 1; i > 0; i--) {
      uint8_t j = random(i + 1);
      bool a = PavIsCSMinus(i);
      bool b = PavIsCSMinus(j);
      PavSetTrialType(i, b);
      PavSetTrialType(j, a);
    }

    // Check max consecutive constraint
    uint8_t maxRun = 1;
    uint8_t run = 1;
    for (uint8_t i = 1; i < total; i++) {
      if (PavIsCSMinus(i) == PavIsCSMinus(i - 1)) {
        run++;
        if (run > maxRun) maxRun = run;
      } else {
        run = 1;
      }
    }

    if (maxRun <= 3) return;  // Valid shuffle
  }
  // After 50 attempts, accept last shuffle (best-effort)
}

// Approximate -ln(U) for U uniform on (0,1] without pulling in libm's log().
// Uses reduction to [1,2) range via repeated halving, then Taylor series.
static float approxNegLnU() {
  uint16_t k = random(1, 10001);
  float x = (float)k;
  float lnK = 0.0f;
  while (x >= 2.0f) {
    x *= 0.5f;
    lnK += 0.6931472f;  // ln(2)
  }
  float d = x - 1.0f;
  lnK += d - 0.5f * d * d + 0.333333f * d * d * d;
  return 9.21034f - lnK;  // ln(10000) - ln(k) = -ln(k/10000)
}

uint32_t PavlovianScheduler::PavSampleIti() {
  float negLnU = approxNegLnU();
  float iti = (float)pavItiMean * negLnU;
  return constrain((uint32_t)iti, pavItiMin, pavItiMax);
}

// --- Trial state machine ---

void PavlovianScheduler::PavTick(uint32_t now) {
  if (pavPhase == PavlovPhase::IDLE) return;

  uint32_t elapsed = now - pavPhaseStart;

  switch (pavPhase) {
    case PavlovPhase::ITI:
      if (elapsed >= pavCurrentIti) {
        PavStartTrial(now);
        pavPhase = PavlovPhase::CUE_ON;
        pavPhaseStart = now;
      }
      break;

    case PavlovPhase::CUE_ON:
      if (elapsed >= pavCueDuration) {
        LogPavlovianEvent(F("TRACE_START"), now);
        pavPhase = PavlovPhase::TRACE;
        pavPhaseStart = now;
      }
      break;

    case PavlovPhase::TRACE:
      if (elapsed >= pavTraceInterval) {
        // Deliver reward or log omission
        if (pavRewardThisTrial) {
          bool isCsMinus = PavIsCSMinus(pavTrialIndex);
          // Select pump: CS+ -> pump, CS- -> pump2 (swapped if counterbalanced)
          Pump* activePump;
          DeviceType pumpType;
          if (isCsMinus != pavCounterbalance) {
            activePump = pump2;
            pumpType = DeviceType::PUMP_2;
          } else {
            activePump = pump;
            pumpType = DeviceType::PUMP;
          }
          if (activePump && activePump->Armed()) {
            activePump->Activate(now, activePump->Duration());
            LogDeviceActivation(pumpType, now, now + activePump->Duration());
          }
          LogPavlovianEvent(F("REWARD_DELIVERED"), now);
        } else {
          LogPavlovianEvent(F("REWARD_OMITTED"), now);
        }
        pavPhase = PavlovPhase::REWARD;
        pavPhaseStart = now;
      }
      break;

    case PavlovPhase::REWARD:
      if (elapsed >= pavConsumptionMs) {
        pavTrialIndex++;
        if (pavTrialIndex >= pavTotalTrials) {
          LogPavlovianEvent(F("ALL_TRIALS_COMPLETE"), now);
          pavPhase = PavlovPhase::IDLE;
        } else {
          pavCurrentIti = PavSampleIti();
          pavPhase = PavlovPhase::ITI;
          pavPhaseStart = now;
        }
      }
      break;

    default:
      break;
  }
}

void PavlovianScheduler::PavStartTrial(uint32_t now) {
  bool isCsMinus = PavIsCSMinus(pavTrialIndex);

  // Resolve reward probability
  uint8_t prob = isCsMinus ? pavCsMinusProb : pavCsPlusProb;
  pavRewardThisTrial = ((uint8_t)random(100) < prob);

  // Select cue: CS+ -> cue, CS- -> cue2 (swapped if counterbalanced)
  Cue* activeCue;
  DeviceType cueType;
  if (isCsMinus != pavCounterbalance) {
    activeCue = cue2;
    cueType = DeviceType::CUE_2;
  } else {
    activeCue = cue;
    cueType = DeviceType::CUE;
  }

  if (activeCue && activeCue->Armed()) {
    activeCue->Activate(now, pavCueDuration);
    LogDeviceActivation(cueType, now, now + pavCueDuration);
  }

  LogPavlovianTrial(now, isCsMinus);
}

// --- Logging ---

void PavlovianScheduler::LogLeverPress(DeviceType source, PressClass cls) {
  SwitchLever* lever = GetLever(source);
  if (!lever) return;

  const __FlashStringHelper* clsStr;
  switch (cls) {
    case PressClass::ACTIVE:   clsStr = F("ACTIVE");   break;
    case PressClass::INACTIVE: clsStr = F("INACTIVE"); break;
    case PressClass::TIMEOUT:  clsStr = F("TIMEOUT");  break;
  }

  Serial.print(F("{\"level\":\"007\",\"device\":\"SWITCH_LEVER\",\"pin\":"));
  Serial.print(lever->Pin());
  Serial.print(F(",\"event\":\"PRESS\",\"class\":\""));
  Serial.print(clsStr);
  Serial.print(F("\",\"start_timestamp\":"));
  Serial.print(lever->StartTimestamp() - sessionOffset);
  Serial.print(F(",\"end_timestamp\":"));
  Serial.print(lever->EndTimestamp() - sessionOffset);
  Serial.print(F(",\"orientation\":\""));
  Serial.print(lever->Orientation());
  Serial.println(F("\"}"));
}

void PavlovianScheduler::LogDeviceActivation(DeviceType target, uint32_t startTs, uint32_t endTs) {
  const __FlashStringHelper* device;
  const __FlashStringHelper* event;
  int8_t pinNum;

  switch (target) {
    case DeviceType::CUE:
      device = F("CUE"); event = F("TONE"); pinNum = cue->Pin(); break;
    case DeviceType::CUE_2:
      device = F("CUE_2"); event = F("TONE"); pinNum = cue2->Pin(); break;
    case DeviceType::PUMP:
      device = F("PUMP"); event = F("INFUSION"); pinNum = pump->Pin(); break;
    case DeviceType::PUMP_2:
      device = F("PUMP_2"); event = F("INFUSION"); pinNum = pump2->Pin(); break;
    default:
      return;
  }

  Serial.print(F("{\"level\":\"007\",\"device\":\""));
  Serial.print(device);
  Serial.print(F("\",\"pin\":"));
  Serial.print(pinNum);
  Serial.print(F(",\"event\":\""));
  Serial.print(event);
  Serial.print(F("\",\"start_timestamp\":"));
  Serial.print(startTs - sessionOffset);
  Serial.print(F(",\"end_timestamp\":"));
  Serial.print(endTs - sessionOffset);
  Serial.println('}');
}

void PavlovianScheduler::LogPavlovianTrial(uint32_t now, bool isCsMinus) {
  Serial.print(F("{\"level\":\"007\",\"device\":\"PAVLOV\",\"event\":\"TRIAL_START\",\"trial\":"));
  Serial.print(pavTrialIndex);
  Serial.print(F(",\"trial_type\":\""));
  Serial.print(isCsMinus ? F("CS_MINUS") : F("CS_PLUS"));
  Serial.print(F("\",\"reward_scheduled\":"));
  Serial.print(pavRewardThisTrial ? F("true") : F("false"));
  Serial.print(F(",\"iti_ms\":"));
  Serial.print(pavCurrentIti);
  Serial.print(F(",\"timestamp\":"));
  Serial.print(now - sessionOffset);
  Serial.println('}');
}

void PavlovianScheduler::LogPavlovianEvent(const __FlashStringHelper* event, uint32_t now) {
  Serial.print(F("{\"level\":\"007\",\"device\":\"PAVLOV\",\"event\":\""));
  Serial.print(event);
  Serial.print(F("\",\"trial\":"));
  Serial.print(pavTrialIndex);
  Serial.print(F(",\"timestamp\":"));
  Serial.print(now - sessionOffset);
  Serial.println(F("\"}"));
}
