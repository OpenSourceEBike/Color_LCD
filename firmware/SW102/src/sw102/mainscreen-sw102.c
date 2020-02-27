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
  .x = 0, .y = 118, \
  .width = 64, .height = -1, \
  .field = &warnField, \
  .border = BorderNone, \
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
Screen mainScreen1 = {
  .onPress = mainScreenOnPress,
	.onEnter = mainScreenOnEnter,
	.onDirtyClean = mainScreenOnDirtyClean,
	.onPostUpdate = mainScreenonPostUpdate,

  .fields = {
    BATTERY_BAR,
    {
      .x = 10, .y = 19,
      .width = 47, // 2 digits
      .height = -1,
      .field = &wheelSpeedIntegerField,
      .font = &BIG_NUMBERS_TEXT_FONT,
      .label_align_x = AlignHidden,
      .align_x = AlignRight,
      .unit_align_x = AlignRight,
      .unit_align_y = AlignTop,
      .show_units = Hide,
      .border = BorderNone,
    },
    {
      .x = 0, .y = 57,
      .width = 0, .height = 34,
      .field = &custom1,
      .font = &SMALL_TEXT_FONT,
      .label_align_x = AlignHidden,
      .align_x = AlignCenter,
      .border = BorderNone,
      .inset_y = 3,
      .show_units = Hide
    },
    {
      .x = 0, .y = 86,
      .width = 0, .height = 28,
      .field = &custom2,
      .font = &MEDIUM_NUMBERS_TEXT_FONT,
      .label_align_x = AlignHidden,
      .align_x = AlignCenter,
      .border = BorderNone,
      .inset_y = 3,
      .show_units = Hide
    },
    STATUS_BAR,
    {
      .field = NULL
    }
  }
};

Screen mainScreen2 = {
  .onPress = mainScreenOnPress,
  .onEnter = mainScreenOnEnter,
  .onDirtyClean = mainScreenOnDirtyClean,
  .onPostUpdate = mainScreenonPostUpdate,

  .fields = {
    BATTERY_BAR,
    {
      .x = 10, .y = 19,
      .width = 47, // 2 digits
      .height = -1,
      .field = &wheelSpeedIntegerField,
      .font = &BIG_NUMBERS_TEXT_FONT,
      .label_align_x = AlignHidden,
      .align_x = AlignRight,
      .unit_align_x = AlignRight,
      .unit_align_y = AlignTop,
      .show_units = Hide,
      .border = BorderNone,
    },
    {
      .x = 0, .y = 57,
      .width = 0, .height = 34,
      .field = &custom3,
      .font = &SMALL_TEXT_FONT,
      .label_align_x = AlignHidden,
      .align_x = AlignCenter,
      .border = BorderNone,
      .inset_y = 3,
      .show_units = Hide
    },
    {
      .x = 0, .y = 86,
      .width = 0, .height = 28,
      .field = &custom4,
      .font = &MEDIUM_NUMBERS_TEXT_FONT,
      .label_align_x = AlignHidden,
      .align_x = AlignCenter,
      .border = BorderNone,
      .inset_y = 3,
      .show_units = Hide
    },
    STATUS_BAR,
    {
      .field = NULL
    }
  }
};

// Show our battery graphic
void battery_display() {
  static uint8_t old_soc = 0xff;

  if (ui8_g_battery_soc != old_soc) {
    old_soc = ui8_g_battery_soc;
    batteryField.rw->dirty = true;
  }
}

void mainScreenOnDirtyClean(void) {
  batteryClearSymbol();
}

void secondMainScreenOnDirtyClean(void) {
  batteryClearSymbol();
}

void mainScreenonPostUpdate(void) {
  UG_DrawLine(0, 62, 63, 62, C_WHITE);
  UG_DrawLine(0, 89, 63, 89, C_WHITE);
  UG_DrawLine(0, 115, 63, 115, C_WHITE);

  UG_DrawLine(0, 62, 0, 115, C_WHITE);
  UG_DrawLine(63, 62, 63, 115, C_WHITE);
}

// Screens in a loop, shown when the user short presses the power button
Screen *screens[] = { &mainScreen1,
		&mainScreen2,
		NULL };


