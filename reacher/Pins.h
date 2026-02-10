#ifndef PINS_H
#define PINS_H

// Input pins
constexpr int8_t PIN_LEVER_RH        = 10;
constexpr int8_t PIN_LEVER_LH        = 12;
constexpr int8_t PIN_LICK_CIRCUIT    = 5;
constexpr int8_t PIN_MICROSCOPE_TS   = 2;   // Frame timestamp ISR (INPUT)

// Output pins
constexpr int8_t PIN_CUE             = 3;   // Tone output (PWM capable)
constexpr int8_t PIN_PUMP            = 4;   // Relay control
constexpr int8_t PIN_LASER           = 6;   // PWM output
constexpr int8_t PIN_MICROSCOPE_TRIG = 9;   // Trigger pulse (OUTPUT)

#endif // PINS_H
