#include "Device.h"
#include "Cue.h"
#include <Arduino.h>

/**
 * @brief Plays a connection or disconnection jingle based on the connection status.
 * 
 * This function generates an audible feedback using tones to indicate whether a device
 * has been linked or unlinked from a GUI. The pitch increases for connection and decreases
 * for disconnection.
 * 
 * @param connected A String indicating the connection status ("LINK" or "UNLINK").
 * @param cue Reference to a Cue object controlling the speaker output pin.
 * @param linkedToGUI Reference to a boolean flag tracking GUI connection status.
 */
void connectionJingle(String connected, Cue& cue, bool& linkedToGUI) {
    if (connected == "LINK") {
        linkedToGUI = true;
        static int32_t pitch = 500; // Starting tone frequency in Hz
        for (int32_t i = 0; i < 3; i++) {
            tone(cue.getPin(), pitch, 100); // Play tone for 100ms
            delay(100);                     // Wait for tone duration
            noTone(cue.getPin());           // Stop tone
            pitch += 500;                   // Increment pitch by 500Hz
        }
        pitch = 500;                        // Reset pitch for next use
        Serial.println("LINKED");           // Log connection status
    } else if (connected == "UNLINK") {
        linkedToGUI = false;
        static int32_t pitch = 1500; // Starting tone frequency in Hz
        for (int32_t i = 0; i < 3; i++) {
            tone(cue.getPin(), pitch, 100); // Play tone for 100ms
            delay(100);                     // Wait for tone duration
            noTone(cue.getPin());           // Stop tone
            pitch -= 500;                   // Decrement pitch by 500Hz
        }
        pitch = 1500;                       // Reset pitch for next use
        Serial.println("UNLINKED");         // Log disconnection status
    }
}

/**
 * @brief Controls cue tone operation.
 * 
 * Turns the cue tone on during its assigned period and off otherwise, if armed.
 * 
 * @param cue Pointer to the Cue object (optional).
 */
void manageCue(Cue* cue) {
    int32_t timestamp = millis();
    if (cue) {
        if (cue->isArmed()) {
            if (timestamp <= cue->getOffTimestamp() && timestamp >= cue->getOnTimestamp()) {
                cue->on(); // Turn on cue
                cue->setRunning(true);
            } else {
                cue->off(); // Turn cue off
                cue->setRunning(false);
            }
        }
    }
}
