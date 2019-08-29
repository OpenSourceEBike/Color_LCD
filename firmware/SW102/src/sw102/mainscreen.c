/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#include <math.h>
#include "stdio.h"
#include "main.h"
#include "utils.h"
#include "screen.h"
#include "rtc.h"
#include "fonts.h"
#include "config.h"
#include "uart.h"
#include "mainscreen.h"
#include "eeprom.h"
#include "buttons.h"
#include "lcd.h"
#include "adc.h"
#include "ugui.h"
#include "configscreen.h"

Field batteryField = FIELD_DRAWTEXT();

static void mainScreenOnEnter() {
	// Set the font preference for this screen
	editable_label_font = &SMALL_TEXT_FONT;
	editable_value_font = &SMALL_TEXT_FONT;
	editable_units_font = &SMALL_TEXT_FONT;

	// Update our graph thresholds based on current values
	motorTempGraph.graph.warn_threshold =
			l2_vars.ui8_motor_temperature_min_value_to_limit;
	motorTempGraph.graph.error_threshold =
			l2_vars.ui8_motor_temperature_max_value_to_limit;
}

/**
 * Appears at the bottom of all screens, includes status msgs or critical fault alerts
 * FIXME - get rid of this nasty define - instead add the concept of Subscreens, so that the battery bar
 * at the top and the status bar at the bottom can be shared across all screens
 */
#define STATUS_BAR \
{ \
    .x = 4, .y = 114, \
    .width = 0, .height = -1, \
    .field = &warnField, \
    .font = &REGULAR_TEXT_FONT, \
}

#define BATTERY_BAR \
    { \
        .x = 0, .y = 0, \
        .width = -1, .height = -1, \
        .field = &batteryField, \
        .font = &MY_FONT_BATTERY, \
    }, \
    { \
        .x = 32, .y = 0, \
        .width = -5, .height = -1, \
        .font = &REGULAR_TEXT_FONT, \
        .field = &socField \
    }
/*
{
    .x = 32, .y = 0,
    .width = -5, .height = -1,
    .field = &tripTimeField
},
*/

//
// Screens
//
Screen mainScreen = {
    .onPress = mainscreen_onpress,
	.onEnter = mainScreenOnEnter,

    .fields = {
    BATTERY_BAR,
    {
        .x = 0, .y = -1,
        .width = 0, .height = -1,
        .field = &assistLevelField,
        .font = &BIG_NUMBERS_TEXT_FONT,
        .modifier = ModNoLabel,
        .border = BorderBottom
    },
    /*
    {
        .x = 19, .y = 16,
        .width = -2, .height = -1,
        // .color = ColorInvert,
        .field = &speedField,
        .border = BorderNone
    }, */
    {
        .x = 0, .y = -3,
        .width = 0, .height = 19,
        .field = &maxPowerField,
        .font = &MEDIUM_NUMBERS_TEXT_FONT,
        .modifier = ModNoLabel,
        .border = BorderBottom
    },
    {
        .x = 0, .y = -3,
        .width = 0, .height = -1,
        .field = &speedField,
        .font = &BIG_NUMBERS_TEXT_FONT,
        .modifier = ModNoLabel,
        .border = BorderNone
    },
    STATUS_BAR,
    {
        .field = NULL
    } }
};

Screen infoScreen = {
    // .onPress = mainscreen_onpress,
	.onEnter = mainScreenOnEnter,

    .fields = {
    BATTERY_BAR,
    {
        .x = 0, .y = -1,
        .width = 0, .height = -1,
        .field = &motorTempField,
        .font = &REGULAR_TEXT_FONT,
        .modifier = ModNoLabel,
        .border = BorderBottom
    },
    {
        .x = 0, .y = -1,
        .width = 0, .height = -1,
        .field = &humanPowerField,
        .font = &REGULAR_TEXT_FONT,
        .modifier = ModNoLabel,
        .border = BorderBottom
    },
    {
        .x = 0, .y = -1,
        .width = 0, .height = -1,
        .field = &tripDistanceField,
        .font = &REGULAR_TEXT_FONT,
        .modifier = ModNoLabel,
        .border = BorderBottom
    },
    {
        .x = 0, .y = -1,
        .width = 0, .height = -1,
        .field = &odoField,
        .font = &REGULAR_TEXT_FONT,
        .modifier = ModNoLabel,
        .border = BorderBottom
    },
    STATUS_BAR,
    {
        .field = NULL
    } }
};

// Screens in a loop, shown when the user short presses the power button
Screen *screens[] = { &mainScreen, &configScreen,
		&infoScreen,
		NULL };


