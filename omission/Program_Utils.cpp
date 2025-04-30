#include "Program_Utils.h"
#include "Device.h"
#include "Lever.h"
#include "LickCircuit.h"
#include "Laser.h"
#include "Laser_Utils.h"
#include "Pump.h"
#include "Pump_Utils.h"
#include "Cue.h"
#include "Cue_Utils.h"

extern uint32_t traceIntervalLength;     ///< Length of the trace interval (ms).
extern uint32_t differenceFromStartTime; ///< Offset from program start time (ms).
extern Lever leverRH, leverLH;           ///< Right and left lever objects.
extern Cue cs;                           ///< Cue object.
extern Pump pump;                        ///< Pump object.
extern LickCircuit lickCircuit;          ///< Lick circuit object.
extern Laser laser;                      ///< Laser object.
extern uint32_t lastInfusionTime;        ///< Marker for the last infusion timestamp. 
extern bool programIsRunning;            ///< Boolean flag if program is running.
extern uint32_t omissionInterval;        ///< Interval for omission duration.

/**
 * @brief Starts the program and triggers imaging.
 * 
 * Signals the start of the program and sets the time offset for timestamps.
 * 
 * @param pin The digital pin to trigger imaging.
 */
void startProgram(byte pin) {
    Serial.println();
    Serial.println("========== PROGRAM START ==========");
    Serial.println();
    Serial.println("START-TIME,ORIGIN,0,0");
    digitalWrite(pin, HIGH);            // Trigger imaging start
    delay(50);                          // Ensure data transmission
    digitalWrite(pin, LOW);             // Finish trigger
    differenceFromStartTime = millis(); // Set program start offset
    lastInfusionTime = millis();        // Initialize last infusion time
}

/**
 * @brief Ends the program and disarms all devices.
 * 
 * Signals the end of the program, stops imaging, and disarms all devices.
 * 
 * @param pin The digital pin to trigger imaging end.
 */
void endProgram(byte pin) {
    int32_t terminus = millis() - differenceFromStartTime;
    String message = "END-TIME,TERMINUS," + String(terminus) + "," + String(terminus);
    Serial.println(message);
    Serial.println();
    Serial.println("========== PROGRAM END ==========");
    Serial.println();
    digitalWrite(pin, HIGH); // Trigger imaging end
    delay(50);
    digitalWrite(pin, LOW);
    leverRH.disarm();
    leverLH.disarm();
    cs.disarm();
    pump.disarm();
    lickCircuit.disarm();
    laser.off();
}

/**
 * @brief Method to encapsulate all managed devices.
 * 
 * Loop calls managing functions for selected devices.
 */
void manageDevices() {
  manageCue(&cs);
  managePump(&pump);
  manageStim(laser);
}

/**
 * @brief Triggers a pump infusion after an omission interval.
 * 
 * Delivers an infusion and cue tone if the omission interval has elapsed without an active press.
 */
void triggerInfusion() {
    uint32_t currentMillis = millis();
    if (!programIsRunning || (currentMillis - lastInfusionTime < omissionInterval)) {
        return;
    }
    if (pump.isArmed()) {
        uint32_t timestamp = currentMillis;

        pump.setInfusionPeriod(timestamp, 0);
        String infusionEntry = "PUMP,INFUSION,";
        infusionEntry += differenceFromStartTime ? String(pump.getInfusionStartTimestamp() - differenceFromStartTime) : String(pump.getInfusionStartTimestamp());
        infusionEntry += ",";
        infusionEntry += differenceFromStartTime ? String(pump.getInfusionEndTimestamp() - differenceFromStartTime) : String(pump.getInfusionEndTimestamp());
        Serial.println(infusionEntry);
        lastInfusionTime = currentMillis;

    }
    if (cs.isArmed()) {    
        cs.setOnTimestamp(currentMillis);
        cs.setOffTimestamp(currentMillis);
    }
    if (laser.isArmed() && laser.getStimMode() == ACTIVE_PRESS) {
        laser.setStimPeriod(currentMillis);
        laser.setStimState(ACTIVE);
    }
};
