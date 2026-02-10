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
extern int32_t variableInterval;            ///< Variable interval for reward delivery.
extern uint32_t differenceFromStartTime;    ///< Offset from program start time (ms).
extern Lever* activeLever;                  ///< Pointer to the active lever.
extern Lever* inactiveLever;                ///< Pointer to the inactive lever.

/**
 * @brief Logs lever press and release data to the serial monitor.
 * 
 * Records press and release timestamps, and infusion data if applicable, adjusted by program start time.
 * 
 * @param lever Reference to a pointer to the Lever object being monitored.
 * @param pump Pointer to the Pump object (optional).
 */
void pressingDataEntry(Lever*& lever, Pump* pump) {
    String pressEntry;
    String infusionEntry;
    lever->setReleaseTimestamp(millis()); // Set press release timestamp
    pressEntry = lever->getOrientation() + F("_LEVER,");
    pressEntry += lever->getPressType() + F("_PRESS,");
    if (differenceFromStartTime) {
        pressEntry += String(lever->getPressTimestamp() - differenceFromStartTime) + ",";
        pressEntry += String(lever->getReleaseTimestamp() - differenceFromStartTime);
    } else {
        pressEntry += String(lever->getPressTimestamp()) + ",";
        pressEntry += String(lever->getReleaseTimestamp());
    }
    Serial.println(pressEntry); // Send press data to serial connection
    if (pump && pump->isArmed() && lever->getPressType() == "ACTIVE") {
        infusionEntry = F("PUMP,INFUSION,");
        infusionEntry += differenceFromStartTime ? String(pump->getInfusionStartTimestamp() - differenceFromStartTime) : String(pump->getInfusionStartTimestamp());
        infusionEntry += ",";
        infusionEntry += differenceFromStartTime ? String(pump->getInfusionEndTimestamp() - differenceFromStartTime) : String(pump->getInfusionEndTimestamp());
        Serial.println(infusionEntry);
    }
}

/**
 * @brief Defines the type of lever press based on variable interval logic.
 * 
 * Labels the press as "ACTIVE" if it occurs within the random interval and no prior active press
 * has occurred, otherwise "INACTIVE" or "NO CONDITION".
 * 
 * @param programRunning Boolean indicating if the program is running.
 * @param lever Reference to a pointer to the Lever object being pressed.
 * @param cue Pointer to the Cue object (optional).
 * @param pump Pointer to the Pump object (optional).
 */
void definePressActivity(bool programRunning, Lever*& lever, Cue* cue, Pump* pump, Laser* laser) {
    int32_t timestamp = millis();
    if (lever == activeLever && !lever->getActivePressOccurred() && 
        timestamp >= lever->getIntervalStartTime() + lever->getRandomInterval() && 
        timestamp < lever->getIntervalStartTime() + variableInterval && cue->isArmed()) {
        lever->setPressType("ACTIVE");
        lever->setActivePressOccurred(true);
        deliverReward(activeLever, cue, pump, laser);
    } else if (!cue->isArmed()) {
        lever->setPressType("NO CONDITION");
    } else {
        lever->setPressType("INACTIVE");
    }
}

/**
 * @brief Monitors lever pressing with debouncing and variable interval logic.
 * 
 * Detects lever presses, applies debouncing, and resets the variable interval when elapsed.
 * 
 * @param programRunning Boolean indicating if the program is running.
 * @param lever Reference to a pointer to the Lever object.
 * @param cue Pointer to the Cue object (optional).
 * @param pump Pointer to the Pump object (optional).
 */
void monitorPressing(bool programRunning, Lever*& lever, Cue* cue, Pump* pump, Laser* laser) {
    static uint32_t lastDebounceTime = 0; // Last time the lever input was toggled
    const uint32_t debounceDelay = 100;   // Debounce time in milliseconds
    int32_t timestamp = millis();
    manageCue(cue);                       // Manage cue delivery
    managePump(pump);                     // Manage infusion delivery
    if (lever->isArmed()) {
        bool currentLeverState = digitalRead(lever->getPin()); // Read current state
        if (currentLeverState != lever->getPreviousLeverState()) {
            lastDebounceTime = timestamp; // Reset debouncing timer
        }
        if ((timestamp - lastDebounceTime) > debounceDelay) {
            if (currentLeverState != lever->getStableLeverState()) {
                lever->setStableLeverState(currentLeverState); // Update stable state
                if (currentLeverState == LOW) { // Lever press detected
                    lever->setPressTimestamp(timestamp);
                    definePressActivity(programRunning, lever, cue, pump, laser);
                } else { // Lever release detected
                    lever->setReleaseTimestamp(timestamp);
                    pressingDataEntry(lever, pump);
                }
            }
        }
        lever->setPreviousLeverState(currentLeverState); // Update previous state
    }
    if (timestamp - lever->getIntervalStartTime() >= variableInterval) {
        long int newStartTime = timestamp;
        lever->resetInterval(variableInterval, newStartTime);
    }
}
