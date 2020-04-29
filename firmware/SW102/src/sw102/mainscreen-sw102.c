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
    .x = 3, .y = 2, \
    .width = -1, .height = -1, \
    .field = &batteryField, \
}, \
{ \
  .x = 38, .y = 2, \
  .width = 24, .height = -1, \
  .align_x = AlignRight, \
  .font = &REGULAR_TEXT_FONT, \
  .field = &socField \
}

//
// Screens
//

#define MAIN_SCREEN_WHEELSPEED_X                  1
#define MAIN_SCREEN_WHEELSPEED_Y                  19
#define MAIN_SCREEN_WHEELSPEED_WIDTH              62
#define MAIN_SCREEN_WHEELSPEED_HEIGHT             40

#define MAIN_SCREEN_WHEELSPEED_ALTERNATE_X        1
#define MAIN_SCREEN_WHEELSPEED_ALTERNATE_Y        21
#define MAIN_SCREEN_WHEELSPEED_ALTERNATE_WIDTH    62
#define MAIN_SCREEN_WHEELSPEED_ALTERNATE_HEIGHT   34

#define MAIN_SCREEN_CUSTOM_1_X                    1
#define MAIN_SCREEN_CUSTOM_1_Y                    63
#define MAIN_SCREEN_CUSTOM_1_WIDTH                62
#define MAIN_SCREEN_CUSTOM_1_HEIGHT               25

#define MAIN_SCREEN_CUSTOM_2_X                    1
#define MAIN_SCREEN_CUSTOM_2_Y                    90
#define MAIN_SCREEN_CUSTOM_2_WIDTH                62
#define MAIN_SCREEN_CUSTOM_2_HEIGHT               25

Screen mainScreen1 = {
  .onPress = mainScreenOnPress,
	.onEnter = mainScreenOnEnter,
	.onDirtyClean = mainScreenOnDirtyClean,
	.onPostUpdate = mainScreenonPostUpdate,

  .fields = {
    BATTERY_BAR,
    {
      .x = MAIN_SCREEN_WHEELSPEED_X,
      .y = MAIN_SCREEN_WHEELSPEED_Y,
      .width = MAIN_SCREEN_WHEELSPEED_WIDTH,
      .height = MAIN_SCREEN_WHEELSPEED_HEIGHT,
      .field = &wheelSpeedIntegerField,
      .font = &BIG_NUMBERS_TEXT_FONT,
      .label_align_x = AlignHidden,
      .align_x = AlignCenter,
      .show_units = Hide,
      .border = BorderNone,
    },
    {
      .x = MAIN_SCREEN_WHEELSPEED_ALTERNATE_X,
      .y = MAIN_SCREEN_WHEELSPEED_ALTERNATE_Y,
      .width = MAIN_SCREEN_WHEELSPEED_ALTERNATE_WIDTH,
      .height = MAIN_SCREEN_WHEELSPEED_ALTERNATE_HEIGHT,
      .field = &fieldAlternate,
      .font = &MEDIUM_NUMBERS_TEXT_FONT,
      .label_align_y = AlignTop,
      .align_x = AlignCenter,
      .inset_y = 6,
      .border = BorderNone,
      .show_units = Hide
    },
    {
      .x = MAIN_SCREEN_CUSTOM_1_X,
      .y = MAIN_SCREEN_CUSTOM_1_Y,
      .width = MAIN_SCREEN_CUSTOM_1_WIDTH,
      .height = MAIN_SCREEN_CUSTOM_1_HEIGHT,
      .field = &custom1,
      .font = &MEDIUM_NUMBERS_TEXT_FONT,
      .label_align_x = AlignHidden,
      .align_x = AlignCenter,
      .border = BorderNone,
      .inset_y = 3,
      .show_units = Hide
    },
    {
      .x = MAIN_SCREEN_CUSTOM_2_X,
      .y = MAIN_SCREEN_CUSTOM_2_Y,
      .width = MAIN_SCREEN_CUSTOM_2_WIDTH,
      .height = MAIN_SCREEN_CUSTOM_2_HEIGHT,
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
      .x = MAIN_SCREEN_WHEELSPEED_X,
      .y = MAIN_SCREEN_WHEELSPEED_Y,
      .width = MAIN_SCREEN_WHEELSPEED_WIDTH,
      .height = MAIN_SCREEN_WHEELSPEED_HEIGHT,
      .field = &wheelSpeedIntegerField,
      .font = &BIG_NUMBERS_TEXT_FONT,
      .label_align_x = AlignHidden,
      .align_x = AlignCenter,
      .show_units = Hide,
      .border = BorderNone,
    },
    {
      .x = MAIN_SCREEN_WHEELSPEED_ALTERNATE_X,
      .y = MAIN_SCREEN_WHEELSPEED_ALTERNATE_Y,
      .width = MAIN_SCREEN_WHEELSPEED_ALTERNATE_WIDTH,
      .height = MAIN_SCREEN_WHEELSPEED_ALTERNATE_HEIGHT,
      .field = &fieldAlternate,
      .font = &MEDIUM_NUMBERS_TEXT_FONT,
      .label_align_y = AlignTop,
      .align_x = AlignCenter,
      .inset_y = 6,
      .border = BorderNone,
      .show_units = Hide
    },
    {
      .x = MAIN_SCREEN_CUSTOM_1_X,
      .y = MAIN_SCREEN_CUSTOM_1_Y,
      .width = MAIN_SCREEN_CUSTOM_1_WIDTH,
      .height = MAIN_SCREEN_CUSTOM_1_HEIGHT,
      .field = &custom3,
      .font = &MEDIUM_NUMBERS_TEXT_FONT,
      .label_align_x = AlignHidden,
      .align_x = AlignCenter,
      .border = BorderNone,
      .inset_y = 3,
      .show_units = Hide
    },
    {
      .x = MAIN_SCREEN_CUSTOM_2_X,
      .y = MAIN_SCREEN_CUSTOM_2_Y,
      .width = MAIN_SCREEN_CUSTOM_2_WIDTH,
      .height = MAIN_SCREEN_CUSTOM_2_HEIGHT,
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

Screen mainScreen3 = {
  .onPress = mainScreenOnPress,
  .onEnter = mainScreenOnEnter,
  .onDirtyClean = mainScreenOnDirtyClean,
  .onPostUpdate = mainScreenonPostUpdate,

  .fields = {
    BATTERY_BAR,
    {
      .x = MAIN_SCREEN_WHEELSPEED_X,
      .y = MAIN_SCREEN_WHEELSPEED_Y,
      .width = MAIN_SCREEN_WHEELSPEED_WIDTH,
      .height = MAIN_SCREEN_WHEELSPEED_HEIGHT,
      .field = &wheelSpeedIntegerField,
      .font = &BIG_NUMBERS_TEXT_FONT,
      .label_align_x = AlignHidden,
      .align_x = AlignCenter,
      .show_units = Hide,
      .border = BorderNone,
    },
    {
      .x = MAIN_SCREEN_WHEELSPEED_ALTERNATE_X,
      .y = MAIN_SCREEN_WHEELSPEED_ALTERNATE_Y,
      .width = MAIN_SCREEN_WHEELSPEED_ALTERNATE_WIDTH,
      .height = MAIN_SCREEN_WHEELSPEED_ALTERNATE_HEIGHT,
      .field = &fieldAlternate,
      .font = &MEDIUM_NUMBERS_TEXT_FONT,
      .label_align_y = AlignTop,
      .align_x = AlignCenter,
      .inset_y = 6,
      .border = BorderNone,
      .show_units = Hide
    },
    {
      .x = MAIN_SCREEN_CUSTOM_1_X,
      .y = MAIN_SCREEN_CUSTOM_1_Y,
      .width = MAIN_SCREEN_CUSTOM_1_WIDTH,
      .height = MAIN_SCREEN_CUSTOM_1_HEIGHT,
      .field = &custom5,
      .font = &MEDIUM_NUMBERS_TEXT_FONT,
      .label_align_x = AlignHidden,
      .align_x = AlignCenter,
      .border = BorderNone,
      .inset_y = 3,
      .show_units = Hide
    },
    {
      .x = MAIN_SCREEN_CUSTOM_2_X,
      .y = MAIN_SCREEN_CUSTOM_2_Y,
      .width = MAIN_SCREEN_CUSTOM_2_WIDTH,
      .height = MAIN_SCREEN_CUSTOM_2_HEIGHT,
      .field = &custom6,
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
  batteryField.rw->dirty = true;

  UG_DrawLine(0, 62, 63, 62, C_WHITE);
  UG_DrawLine(0, 89, 63, 89, C_WHITE);
  UG_DrawLine(0, 115, 63, 115, C_WHITE);

  UG_DrawLine(0, 63, 0, 114, C_WHITE);
  UG_DrawLine(63, 63, 63, 114, C_WHITE);

  // find if the next lines should be draw or not (white color to draw them)
  UG_COLOR street_mode_color = C_BLACK;
  if (ui_vars.ui8_street_mode_function_enabled &&
      ui_vars.ui8_street_mode_enabled) {
    street_mode_color = C_WHITE;
  }

  UG_DrawLine(0, 0, 63, 0, street_mode_color);
  UG_DrawLine(0, 0, 0, 61, street_mode_color);
  UG_DrawLine(63, 0, 63, 61, street_mode_color);
}

void secondMainScreenOnDirtyClean(void) {
  batteryClearSymbol();
}

void mainScreenonPostUpdate(void) {
}

// Screens in a loop, shown when the user short presses the power button
Screen *screens[] = { &mainScreen1, &mainScreen2, &mainScreen3,
		NULL };


