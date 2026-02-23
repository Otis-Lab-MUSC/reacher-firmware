/**
 * @file Microscope.h
 * @brief Two-photon microscope sync via trigger pulses and frame-signal ISR.
 * @ingroup devices
 */

#ifndef MICROSCOPE_H
#define MICROSCOPE_H

#include <Arduino.h>

/// @brief Two-photon microscope sync via trigger pulses and frame-signal ISR.
///
/// Not derived from Device. Manages two pins: trigger (output) and timestamp
/// (ISR input on INT0, pin 2).
/// @warning All ISR-accessed fields must be volatile. 32-bit reads require
/// interrupt protection on AVR.
class Microscope {
public:
  Microscope(int8_t triggerPin, int8_t timestampPin);

  /// @brief Static ISR handler for INT0 rising edge. Captures millis() timestamp.
  static void TimestampISR();

  /// @brief Process ISR-captured frame timestamps — call from loop().
  void HandleFrameSignal();

  void ArmToggle(bool armed);
  void SetOffset(uint32_t offset);

  /// @brief Send a digital trigger pulse to the microscope (50ms HIGH).
  void Trigger();

  bool Armed() const;
  byte TriggerPin() const;
  byte TimestampPin() const;

private:
  int8_t triggerPin;              ///< Output pin for trigger pulse
  int8_t timestampPin;            ///< ISR input pin (INT0, pin 2)
  volatile bool received;         ///< ISR flag: true when new frame signal captured
  bool armed;                     ///< True when microscope logging is active
  volatile uint32_t timestamp;    ///< ISR-captured frame timestamp (session-relative)
  volatile uint32_t offset;       ///< Session start offset (volatile — read in ISR)

  static Microscope* instance;    ///< Singleton for ISR dispatch

  /// @brief Serialize frame timestamp to serial JSON (level 008).
  void LogOutput(uint32_t ts);
};

#endif // MICROSCOPE_H
