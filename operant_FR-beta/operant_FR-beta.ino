#include <Arduino.h>
#include <SoftwareSerial.h>
#include "SwitchLever.h"
#include "EventHandler.h"

#define SKETCH_NAME "operant_FR.ino"
#define VERSION "v1.0.1"
#define BAUDRATE 115200

#define RH_LEVER_PIN 10
#define LH_LEVER_PIN 13

SwitchLever rLever(RH_LEVER_PIN, "RH", true);
SwitchLever lLever(LH_LEVER_PIN, "LH", false);

void setup() {
  Serial.begin(BAUDRATE);
  delay(100);
  rLever.ArmToggle();
  lLever.ArmToggle();
}

void loop() {
  rLever.Monitor();
  lLever.Monitor();
}

void Program() {
  
}
