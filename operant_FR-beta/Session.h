#include <Arduino.h>

#ifndef SESSION_H
#define SESSION_H

class Session {
public:
  Session();

  void SetStartTimestamp(uint32_t startTimestamp);
  void SetEndTimestamp(uint32_t endTimestamp);
  
private:
  uint32_t startTimestamp;
  uint32_t endTimestamp;

};

#endif // SESSION_H
