#pragma once

#include "screen.h"

void main_idle(); // call every 20ms
bool mainscreen_onpress(buttons_events_t events);
void showNextScreen();

extern Screen mainScreen, infoScreen, bootScreen;

extern Screen *screens[];

extern Field
  socField,
  timeField,
  assistLevelField,
  wheelSpeedIntegerField,
  wheelSpeedDecimalField,
  tripTimeField,
  tripDistanceField,
  odoField,
  wheelSpeedField,
  cadenceField,
  humanPowerField,
  batteryPowerField,
  batteryVoltageField,
  batteryCurrentField,
  motorCurrentField,
  batterySOCField,
  motorTempField,
  motorErpsField,
  pwmDutyField,
  motorFOCField,
  motorTempGraph,
  graphs,
	custom1, custom2, custom3, custom4,
  bootStatus2,
#ifdef SW102
  warnField; // just close previous definition
#endif
#ifndef SW102 // we don't have any graphs yet on SW102, possibly move this into mainscreen_850.c
  warnField,

  wheelSpeedGraph,
  tripDistanceGraph,
  odoGraph,
  cadenceGraph,
  humanPowerGraph,
  batteryPowerGraph,
  batteryVoltageGraph,
  batteryCurrentGraph,
  motorCurrentGraph,
  batterySOCGraph,
  motorTempGraph,
  motorErpsGraph,
  pwmDutyGraph,
  motorFOCGraph;
#endif

extern Field batteryField; // These fields are custom for board type
void battery_display(); // 850C and sw102 provide alternative versions due to different implementations
void set_conversions();
bool anyscreen_onpress(buttons_events_t events);
void clock_time(void);
void onSetConfigurationClockHours(uint32_t v);
void onSetConfigurationClockMinutes(uint32_t v);
void onSetConfigurationDisplayLcdBacklightOnBrightness(uint32_t v);
void onSetConfigurationDisplayLcdBacklightOffBrightness(uint32_t v);
void onSetConfigurationBatteryTotalWh(uint32_t v);
void onSetConfigurationWheelOdometer(uint32_t v);
void mainScreenonDirtyClean(void);

/// set to true if this boot was caused because we had a watchdog failure, used to show user the problem in the fault line
extern bool wd_failure_detected;

extern uint8_t ui8_g_configuration_clock_hours;
extern uint8_t ui8_g_configuration_clock_minutes;
