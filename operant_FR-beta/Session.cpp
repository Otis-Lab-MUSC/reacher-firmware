#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "Session.h"

Session::Session() {

}

void Session::SetStartTimestamp(uint32_t ts) {
  JsonDocument json;
  String desc;
  startTimestamp = ts;
    
  desc = F("Start timestamp ");
  desc += startTimestamp;

  json["level"] = F("output");
  json["desc"] = desc;

  serializeJsonPretty(json, Serial);
  Serial.println();
}

void Session::SetEndTimestamp(uint32_t ts) {
  JsonDocument json;
  String desc;
  endTimestamp = ts;
    
  desc = F("End timestamp ");
  desc += endTimestamp;

  json["level"] = F("output");
  json["desc"] = desc;

  serializeJsonPretty(json, Serial);
  Serial.println();  
}
