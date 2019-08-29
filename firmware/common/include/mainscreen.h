#pragma once

#include "screen.h"

void mainscreen_show();
void main_idle(); // call every 20ms
bool mainscreen_onpress(buttons_events_t events);

extern Screen mainScreen, infoScreen, bootScreen;

extern Screen *screens[];

extern Field socField,
	timeField,
	assistLevelField,
	wheelSpeedIntegerField,
	wheelSpeedDecimalField,
	maxPowerField,
	humanPowerField,
	warnField,
	tripTimeField,
	tripDistanceField,
	odoField,
	motorTempField,
	pwmDutyField,
	motorErpsField,
	motorFOCField,
	cadenceField;

extern Field batteryField; // These fields are custom for board type
void battery_display(); // 850C and sw102 provide alternative versions due to different implementations
