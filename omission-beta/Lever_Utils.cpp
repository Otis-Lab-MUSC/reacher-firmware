#include "Lever.h"
#include "Cue.h"
#include "Cue_Utils.h"
#include "Pump.h"
#include "Pump_Utils.h"
#include "Laser.h"
#include "Program_Utils.h"
#include <Arduino.h>

extern uint32_t timeoutIntervalStart;       ///< Start time of the timeout interval (ms).
extern uint32_t timeoutIntervalEnd;         ///< End time of the timeout interval (ms).
extern uint32_t timeoutIntervalLength;      ///< Length of the timeout interval (ms).
extern int32_t pressCount;                  ///< Counter for lever presses.
extern int32_t fRatio;                      ///< Fixed ratio for reward delivery.
extern uint32_t differenceFromStartTime;    ///< Offset from program start time (ms).
extern Lever* activeLever;                  ///< Pointer to the active lever.
extern Lever* inactiveLever;                ///< Pointer to the inactive lever.
extern uint32_t lastInfusionTime;        ///< Marker for the last infusion timestamp. 
extern bool programIsRunning;            ///< Boolean flag if program is running.

/**
 * @brief Logs lever press and release data to the serial monitor.
 * 
 * Records the press and release timestamps of a lever, adjusted by the program start time if applicable.
 * 
 * @param lever Reference to a pointer to the Lever object being monitored.
 * @param pump Pointer to the Pump object (optional, can be nullptr).
 */
void pressingDataEntry(Lever*& lever) {
    String pressEntry;
    lever->setReleaseTimestamp(millis()); // Set press release timestamp
    pressEntry = lever->getOrientation() + "_LEVER,";
    pressEntry += lever->getPressType() + "_PRESS,";
    if (differenceFromStartTime) {
        pressEntry += String(lever->getPressTimestamp() - differenceFromStartTime) + ",";
        pressEntry += String(lever->getReleaseTimestamp() - differenceFromStartTime);
    } else {
        pressEntry += String(lever->getPressTimestamp()) + ",";
        pressEntry += String(lever->getReleaseTimestamp());
    }
    Serial.println(pressEntry); // Send data to serial connection
}

/**
 * @brief Defines the type of lever press and triggers associated actions.
 * 
 * Determines whether a lever press is "ACTIVE", "TIMEOUT", or "INACTIVE" based on timing and device states,
 * and triggers rewards or updates counters accordingly.
 * 
 * @param programRunning Boolean indicating if the program is running.
 * @param lever Reference to a pointer to the Lever object being pressed.
 */
void definePressActivity(bool programRunning, Lever*& lever) {
    if (lever == activeLever) {
        lever->setPressType("ACTIVE");
    } else {
        lever->setPressType("INACTIVE");
    }
}

/**
 * @brief Monitors lever pressing with debouncing and resets omission timer.
 * 
 * Detects lever presses, applies debouncing, and resets the infusion timer on active presses.
 * 
 * @param programRunning Boolean indicating if the program is running.
 * @param lever Reference to a pointer to the Lever object.
 */
void monitorPressing(bool programRunning, Lever*& lever) {
    static uint32_t lastDebounceTime = 0; // Last time the lever input was toggled
    const uint32_t debounceDelay = 100;   // Debounce time in milliseconds
    if (lever->isArmed()) {
        bool currentLeverState = digitalRead(lever->getPin()); // Read current state
        if (currentLeverState != lever->getPreviousLeverState()) {
            lastDebounceTime = millis(); // Reset debouncing timer
        }
        if ((millis() - lastDebounceTime) > debounceDelay) {
            if (currentLeverState != lever->getStableLeverState()) {
                lever->setStableLeverState(currentLeverState); // Update stable state
                if (currentLeverState == LOW) { // Lever press detected
                    lever->setPressTimestamp(millis());
                    definePressActivity(programRunning, lever);
                    if (lever->getPressType() == "ACTIVE") {
                        lastInfusionTime = millis(); // Reset omission timer
                    }
                } else { // Lever release detected
                    lever->setReleaseTimestamp(millis());
                    pressingDataEntry(lever);
                }
            }
        }
        lever->setPreviousLeverState(currentLeverState); // Update previous state
    }
}
