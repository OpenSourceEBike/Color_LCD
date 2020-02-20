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
#include "uart.h"
#include "mainscreen.h"
#include "eeprom.h"
#include "buttons.h"
#include "lcd.h"
#include "adc.h"
#include "ugui.h"
#include "configscreen.h"
#include "battery_gui.h"

Field batteryField = FIELD_CUSTOM(renderBattery);

static void mainScreenOnEnter() {
	// Set the font preference for this screen
	editable_label_font = &SMALL_TEXT_FONT;
	editable_value_font = &SMALL_TEXT_FONT;
	editable_units_font = &SMALL_TEXT_FONT;
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
}, \
{ \
  .x = 32, .y = 0, \
  .width = -5, .height = -1, \
  .font = &REGULAR_TEXT_FONT, \
  .field = &socField \
}

//
// Screens
//
Screen mainScreen = {
  .onPress = mainscreen_onpress,
	.onEnter = mainScreenOnEnter,
	.onDirtyClean = mainScreenonDirtyClean,

    .fields = {
    BATTERY_BAR,
    {
        .x = 0, .y = -2,
        .width = 0, .height = -1,
        .field = &wheelSpeedIntegerField,
        .font = &BIG_NUMBERS_TEXT_FONT,
        .label_align_x = AlignHidden,
        .border = BorderBottom,
        .show_units = Hide
    },
    {
        .x = 0, .y = -2,
        .width = 0, .height = 19,
        .field = &assistLevelField,
        .font = &MEDIUM_NUMBERS_TEXT_FONT,
        .label_align_x = AlignHidden,
        .border = BorderBottom | BorderLeft | BorderRight,
        .show_units = Hide
    },
    {
        .x = 0, .y = -2,
        .width = 0, .height = 19,
        .field = &humanPowerField,
        .font = &MEDIUM_NUMBERS_TEXT_FONT,
        .label_align_x = AlignHidden,
        .border = BorderLeft | BorderRight,
        .show_units = Hide
    },
    {
        .x = 0, .y = -2,
        .width = 0, .height = 19,
        .field = &batteryPowerField,
        .font = &MEDIUM_NUMBERS_TEXT_FONT,
        .label_align_x = AlignHidden,
        .border = BorderLeft | BorderRight | BorderBottom,
        .show_units = Hide
    },

    STATUS_BAR,
    {
        .field = NULL
    }
  }
};

Screen infoScreen = {
    // .onPress = mainscreen_onpress,
	.onEnter = mainScreenOnEnter,
  .onCustomized = eeprom_write_variables,
  .onPress = anyscreen_onpress,
  .onDirtyClean = mainScreenonDirtyClean,

    .fields = {
    BATTERY_BAR,
    {
        .x = 0, .y = -3,
        .width = 0, .height = -1,
        .field = &custom1,
        .font = &MEDIUM_NUMBERS_TEXT_FONT,
        .label_align_y = AlignTop,
        .border = BorderBottom | BorderTop
    },
    {
        .x = 0, .y = -3,
        .width = 0, .height = -1,
        .field = &custom2,
        .font = &MEDIUM_NUMBERS_TEXT_FONT,
        .label_align_y = AlignTop,
        .border = BorderBottom
    },
    {
        .x = 0, .y = -3,
        .width = 0, .height = -1,
        .field = &custom3,
        .font = &MEDIUM_NUMBERS_TEXT_FONT,
        .label_align_y = AlignTop,
        .border = BorderBottom
    },
#if 0
    {
        .x = 0, .y = -3,
        .width = 0, .height = -1,
        .field = &custom4,
        .font = &MEDIUM_NUMBERS_TEXT_FONT,
        .label_align_x = AlignHidden,
        .border = BorderBottom
    },
#endif
    STATUS_BAR,
    {
        .field = NULL
    } }
};

// Show our battery graphic
void battery_display() {
  static uint8_t old_soc = 0xff;

  if (ui8_g_battery_soc != old_soc) {
    old_soc = ui8_g_battery_soc;
    batteryField.rw->dirty = true;
  }
}

void mainScreenonDirtyClean(void) {
  batteryClearSymbol();
}

// Screens in a loop, shown when the user short presses the power button
Screen *screens[] = { &mainScreen,
		&infoScreen,
		NULL };


