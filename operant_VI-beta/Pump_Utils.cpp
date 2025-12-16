#include "Pump.h"
#include <Arduino.h>

/**
 * @brief Controls pump operation.
 * 
 * Turns the pump on during the infusion period and off otherwise, if armed.
 * 
 * @param pump Pointer to the Pump object (optional).
 */
void managePump(Pump* pump) {
    int32_t timestamp = millis();
    if (pump->isArmed()) {
        if (timestamp <= pump->getInfusionEndTimestamp() && timestamp >= pump->getInfusionStartTimestamp()) {
            pump->on(); // Turn the pump on
            pump->setRunning(true);
        } else {
            pump->off(); // Turn the pump off
            pump->setRunning(false);
        }
    }
}
