/**
 * @file ReacherHelpers.cpp
 * @brief Shared session helpers used by all REACHER sketch variants.
 */

#include "ReacherHelpers.h"
#include "SwitchLever.h"
#include "Cue.h"
#include "Pump.h"
#include "Laser.h"
#include "LickCircuit.h"
#include "Microscope.h"

void setDeviceTimestampOffset(DeviceSet& ds, uint32_t ts) {
  ds.rLever->SetOffset(ts);
  ds.lLever->SetOffset(ts);
  ds.cue->SetOffset(ts);
  ds.cue2->SetOffset(ts);
  ds.pump->SetOffset(ts);
  ds.pump2->SetOffset(ts);
  ds.lickCircuit->SetOffset(ts);
  if (ds.laser) ds.laser->SetOffset(ts);
  ds.microscope->SetOffset(ts);
}

void armToggleDevices(DeviceSet& ds, bool toggle) {
  ds.rLever->ArmToggle(toggle);
  ds.lLever->ArmToggle(toggle);
  ds.cue->ArmToggle(toggle);
  ds.cue2->ArmToggle(toggle);
  ds.pump->ArmToggle(toggle);
  ds.pump2->ArmToggle(toggle);
  ds.lickCircuit->ArmToggle(toggle);
  if (ds.laser) ds.laser->ArmToggle(toggle);
  ds.microscope->ArmToggle(toggle);
}

void reportDeviceConfig(const __FlashStringHelper* dev, bool armed,
                        uint32_t freq, uint32_t dur) {
  Serial.print(F("{\"level\":\"000\",\"device\":\""));
  Serial.print(dev);
  Serial.print(F("\",\"armed\":"));
  Serial.print(armed ? F("true") : F("false"));
  Serial.print(F(",\"frequency\":"));
  Serial.print(freq);
  Serial.print(F(",\"duration\":"));
  Serial.print(dur);
  Serial.println('}');
}

void reportDeviceConfig(const __FlashStringHelper* dev, bool armed,
                        uint32_t dur) {
  Serial.print(F("{\"level\":\"000\",\"device\":\""));
  Serial.print(dev);
  Serial.print(F("\",\"armed\":"));
  Serial.print(armed ? F("true") : F("false"));
  Serial.print(F(",\"duration\":"));
  Serial.print(dur);
  Serial.println('}');
}

void reportDeviceConfig(const __FlashStringHelper* dev, bool armed) {
  Serial.print(F("{\"level\":\"000\",\"device\":\""));
  Serial.print(dev);
  Serial.print(F("\",\"armed\":"));
  Serial.print(armed ? F("true") : F("false"));
  Serial.println('}');
}

void reportDeviceLever(const __FlashStringHelper* dev, bool armed,
                       bool reinforced) {
  Serial.print(F("{\"level\":\"000\",\"device\":\""));
  Serial.print(dev);
  Serial.print(F("\",\"armed\":"));
  Serial.print(armed ? F("true") : F("false"));
  Serial.print(F(",\"reinforced\":"));
  Serial.print(reinforced ? F("true") : F("false"));
  Serial.println('}');
}
