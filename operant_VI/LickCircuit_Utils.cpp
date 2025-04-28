#include "LickCircuit.h"
#include <Arduino.h>

extern uint32_t differenceFromStartTime; ///< Offset from program start time (ms).

/**
 * @brief Monitors licking activity with debouncing.
 * 
 * Detects lick events, applies debouncing, and logs timestamps.
 * 
 * @param lickSpout Reference to the LickCircuit object.
 */
void monitorLicking(LickCircuit& lickSpout) {
    static uint32_t lastDebounceTime = 0; // Last time the lick input was toggled
    const uint32_t debounceDelay = 25;    // Debounce time in milliseconds
    if (lickSpout.isArmed()) {
        bool currentLickState = digitalRead(lickSpout.getPin());
        if (currentLickState != lickSpout.getPreviousLickState()) {
            lastDebounceTime = millis();
        }
        if ((millis() - lastDebounceTime) > debounceDelay) {
            if (currentLickState != lickSpout.getStableLickState()) {
                lickSpout.setStableLickState(currentLickState);
                if (currentLickState == HIGH) {
                    lickSpout.setLickTouchTimestamp(millis());
                } else {
                    lickSpout.setLickReleaseTimestamp(millis());
                    String lickEntry = "LICK_CIRCUIT,LICK," +
                                       String(lickSpout.getLickTouchTimestamp() - differenceFromStartTime) + "," +
                                       String(lickSpout.getLickReleaseTimestamp() - differenceFromStartTime);
                    Serial.println(lickEntry);
                }
            }
        }
        lickSpout.setPreviousLickState(currentLickState);
    }
}
