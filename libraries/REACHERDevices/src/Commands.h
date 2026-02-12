#ifndef COMMANDS_H
#define COMMANDS_H

// REACHER Serial Command Codes
//
// Encoding: [Device Prefix][Action Suffix]
//   Prefix:  1xx=Controller  2xx=Session  3xx=Cue  4xx=Pump
//            5xx=Lick  6xx=Laser  9xx=Microscope
//            10xx=RH Lever  13xx=LH Lever
//   Suffix:  x00=disarm  x01=arm  x03=test
//            x71=set frequency  x72=set duration
//            x74=set timeout  x75=set ratio
//            x80=set inactive/mode B  x81=set active/mode A  x82=set mode B
//
// Output JSON levels (firmware -> host):
//   "000" — Configuration / settings dump
//   "001" — Arm / disarm state changes
//   "006" — Error messages
//   "007" — Behavioral events
//   "008" — Microscope frame timestamps

namespace Cmd {
  // --- Controller (1xx) ---
  constexpr int SESSION_END          = 100;
  constexpr int SESSION_START        = 101;
  constexpr int IDENTIFY             = 102;
  constexpr int TEST_CHAIN           = 103;
  constexpr int TEST_MODE            = 104;
  constexpr int SESSION_PAUSE        = 105;  // Pause/resume active session

  // --- Session Setup (2xx) ---
  constexpr int SET_RATIO            = 201;
  constexpr int SET_PARADIGM         = 202;
  constexpr int SET_OMISSION_INTERVAL = 203;
  constexpr int SET_VI_INTERVAL      = 204;
  constexpr int SET_PR_STEP          = 205;
  constexpr int SET_TRACE_INTERVAL   = 220;

  // Pavlovian parameters (206-219)
  constexpr int PAV_CS_PLUS_PROB     = 206;
  constexpr int PAV_CS_MINUS_PROB    = 207;
  constexpr int PAV_CS_PLUS_COUNT    = 208;
  constexpr int PAV_CS_MINUS_COUNT   = 209;
  constexpr int PAV_CS_PLUS_FREQ     = 210;
  constexpr int PAV_CS_MINUS_FREQ    = 211;
  constexpr int PAV_COUNTERBALANCE   = 212;
  constexpr int PAV_CUE_DURATION     = 213;
  constexpr int PAV_TRACE_INTERVAL   = 214;
  constexpr int PAV_CONSUMPTION      = 215;
  constexpr int PAV_ITI_MEAN         = 216;
  constexpr int PAV_ITI_MIN          = 217;
  constexpr int PAV_ITI_MAX          = 218;
  constexpr int PAV_PULSE_CONFIG     = 219;

  // --- Cue (3xx) ---
  constexpr int CUE_DISARM           = 300;
  constexpr int CUE_ARM              = 301;
  constexpr int CUE2_DISARM          = 310;
  constexpr int CUE_TEST             = 303;
  constexpr int CUE2_ARM             = 311;
  constexpr int CUE2_TEST            = 313;
  constexpr int CUE_SET_FREQUENCY    = 371;
  constexpr int CUE_SET_DURATION     = 372;
  constexpr int CUE_SET_TRACE        = 373;  // deprecated
  constexpr int CUE2_SET_FREQUENCY   = 381;
  constexpr int CUE2_SET_DURATION    = 382;

  // --- Pump (4xx) ---
  constexpr int PUMP_DISARM          = 400;
  constexpr int PUMP_ARM             = 401;
  constexpr int PUMP2_DISARM         = 410;
  constexpr int PUMP_TEST            = 403;
  constexpr int PUMP2_ARM            = 411;
  constexpr int PUMP2_TEST           = 413;
  constexpr int PUMP_SET_DURATION    = 472;
  constexpr int PUMP_SET_TRACE       = 473;  // deprecated
  constexpr int PUMP2_SET_DURATION   = 482;

  // --- Lick Circuit (5xx) ---
  constexpr int LICK_DISARM          = 500;
  constexpr int LICK_ARM             = 501;

  // --- Laser (6xx) ---
  constexpr int LASER_DISARM         = 600;
  constexpr int LASER_ARM            = 601;
  constexpr int LASER_TEST           = 603;
  constexpr int LASER_SET_FREQUENCY  = 671;
  constexpr int LASER_SET_DURATION   = 672;
  constexpr int LASER_SET_TRACE      = 673;  // deprecated
  constexpr int LASER_MODE_CONTINGENT  = 681;
  constexpr int LASER_MODE_INDEPENDENT = 682;

  // --- Microscope (9xx) ---
  constexpr int MICROSCOPE_DISARM    = 900;
  constexpr int MICROSCOPE_ARM       = 901;
  constexpr int MICROSCOPE_TEST      = 903;

  // --- RH Lever (10xx) ---
  constexpr int LEVER_RH_DISARM      = 1000;
  constexpr int LEVER_RH_ARM         = 1001;
  constexpr int LEVER_RH_SET_TIMEOUT = 1074;
  constexpr int LEVER_RH_SET_RATIO   = 1075;
  constexpr int LEVER_RH_SET_INACTIVE = 1080;
  constexpr int LEVER_RH_SET_ACTIVE  = 1081;

  // --- LH Lever (13xx) ---
  constexpr int LEVER_LH_DISARM      = 1300;
  constexpr int LEVER_LH_ARM         = 1301;
  constexpr int LEVER_LH_SET_TIMEOUT = 1374;
  constexpr int LEVER_LH_SET_RATIO   = 1375;
  constexpr int LEVER_LH_SET_INACTIVE = 1380;
  constexpr int LEVER_LH_SET_ACTIVE  = 1381;
}

#endif // COMMANDS_H
