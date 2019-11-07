#pragma once

#include "screen.h"

// number of GraphData objects in array
#ifndef SW102
// @casainho I see you removed the concept of a cache of current graph data.  Which is okay, but unfortunate for
// the much more limited RAM of the SW102.  I had hoped to allow an unlimited number of graph types on all platforms
// but only keep a cache of points for the graphs the user is actually showing.  By removing that cache concept
// we can only allow one graph type on the SW102 (the first one).
// Someday I might add back the cache code, so that graphs work a bit better on memory constrained devices.
#define GRAPHS_GRAPH_DATA_SIZE 13
#else
#define GRAPHS_GRAPH_DATA_SIZE 1
#endif

void mainscreen_show();
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
  batterySOCField,
  motorTempField,
  motorErpsField,
  pwmDutyField,
  motorFOCField,
  motorTempGraph,
  graphs,
	custom1, custom2, custom3, custom4,
	warnField;

extern GraphData graphsGraphData[GRAPHS_GRAPH_DATA_SIZE];

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

/// set to true if this boot was caused because we had a watchdog failure, used to show user the problem in the fault line
extern bool wd_failure_detected;

extern uint8_t ui8_g_configuration_clock_hours;
extern uint8_t ui8_g_configuration_clock_minutes;
