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
#include "battery_gui.h"


Field batteryField = FIELD_CUSTOM(renderBattery);

// We currently don't have any graphs in the SW102, so leave them here until then
Field humanPowerGraph = FIELD_GRAPH(&humanPowerField);
Field speedGraph = FIELD_GRAPH(&wheelSpeedIntegerField);
Field motorTempGraph = FIELD_GRAPH(&motorTempField);
Field motorPowerGraph = FIELD_GRAPH(&maxPowerField);
Field pwmDutyGraph = FIELD_GRAPH(&pwmDutyField);
Field motorErpsGraph = FIELD_GRAPH(&motorErpsField);
Field motorFOCGraph = FIELD_GRAPH(&motorFOCField);
Field cadenceGraph = FIELD_GRAPH(&cadenceField);
Field batteryVoltageGraph = FIELD_GRAPH(&batteryVoltageField, .min_threshold = -1, .warn_threshold = -1, .error_threshold = -1);


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

static void mainScreenOnDirtyClean() {
  // main screen mask
  // horizontal lines
  UG_DrawLine(0, 33, 319, 33, MAIN_SCREEN_FIELD_LABELS_COLOR);
  UG_DrawLine(0, 155, 319, 155, MAIN_SCREEN_FIELD_LABELS_COLOR);
  UG_DrawLine(0, 235, 319, 235, MAIN_SCREEN_FIELD_LABELS_COLOR);
  UG_DrawLine(0, 315, 319, 315, MAIN_SCREEN_FIELD_LABELS_COLOR);

  // vertical line
  UG_DrawLine(159, 156, 159, 314, MAIN_SCREEN_FIELD_LABELS_COLOR);

  UG_SetBackcolor(C_BLACK);
  UG_SetForecolor(MAIN_SCREEN_FIELD_LABELS_COLOR);
  UG_FontSelect(&FONT_10X16);
  UG_PutString(12, 46, "ASSIST");

  // wheel speed
  if(l3_vars.ui8_units_type == 0)
  {
    UG_PutString(260, 46 , "KM/H");
  }
  else
  {
    UG_PutString(265, 46 , "MPH");
  }
}

void mainScreenOnPostUpdate(void) {
  // wheel speed print dot
  UG_FillCircle(247, 128, 3, C_WHITE);
}

/**
 * Appears at the bottom of all screens, includes status msgs or critical fault alerts
 * FIXME - get rid of this nasty define - instead add the concept of Subscreens, so that the battery bar
 * at the top and the status bar at the bottom can be shared across all screens
 */
#define STATUS_BAR \
{ \
    .x = 4, .y = SCREEN_HEIGHT - 18, \
    .width = 0, .height = -1, \
    .field = &warnField, \
    .font = &SMALL_TEXT_FONT, \
}

#define BATTERY_BAR \
  { \
      .x = 0, .y = 0, \
      .width = -1, .height = -1, \
      .field = &batteryField, \
      .font = &MY_FONT_BATTERY, \
  }, \
  { \
      .x = 8 + ((7 + 1 + 1) * 10) + (1 * 2) + 10, .y = 2, \
      .width = -5, .height = -1, \
      .font = &REGULAR_TEXT_FONT, \
      .field = &socField \
  }, \
	{ \
		.x = 228, .y = 2, \
		.width = -5, .height = -1, \
		.font = &REGULAR_TEXT_FONT, \
		.field = &timeField \
	}

//
// Screens
//
Screen mainScreen = {
  .onPress = mainscreen_onpress,
  .onEnter = mainScreenOnEnter,
  .onDirtyClean = mainScreenOnDirtyClean,
  .onPostUpdate = mainScreenOnPostUpdate,

  .fields = {
    BATTERY_BAR,
    {
      .x = 20, .y = 77,
      .width = 45, .height = -1,
      .field = &assistLevelField,
      .font = &BIG_NUMBERS_TEXT_FONT,
      .label_align_x = AlignHidden,
      .align_x = AlignCenter,
      .unit_align_x = AlignRight,
      .unit_align_y = AlignTop,
      .border = BorderNone,
    },
    {
      .x = 119, .y = 56,
      .width = 123, // 2 digits
      .height = 99,
      .field = &wheelSpeedIntegerField,
      .font = &HUGE_NUMBERS_TEXT_FONT,
      .label_align_x = AlignHidden,
      .align_x = AlignRight,
      .unit_align_x = AlignRight,
      .unit_align_y = AlignTop,
      .border = BorderNone,
    },
    {
      .x = 253, .y = 77,
      .width = 45, // 1 digit
      .height = 72,
      .field = &wheelSpeedDecimalField,
      .font = &BIG_NUMBERS_TEXT_FONT,
      .label_align_x = AlignHidden,
      .align_x = AlignCenter,
      .unit_align_x = AlignCenter,
      .unit_align_y = AlignTop,
      .border = BorderNone,
    },
    {
      .x = 1, .y = 161,
      .width = XbyEighths(4) - 4,
      .height = 72,
      .align_x = AlignRight,
      .inset_y = 12,
      .inset_x = 16, // space for 5 digits
      .field = &tripDistanceField,
      .font = &MEDIUM_NUMBERS_TEXT_FONT,
      .label_align_y = AlignTop,
      .border = BorderNone,
    },
    {
      .x = XbyEighths(4) + 1, .y = 161,
      .width = XbyEighths(4) - 4,
      .height = 72,
      .align_x = AlignRight,
      .inset_y = 12,
      .inset_x = 28, // space for 4 digits
      .field = &maxPowerField,
      .font = &MEDIUM_NUMBERS_TEXT_FONT,
      .label_align_y = AlignTop,
      .border = BorderNone,
    },
    {
      .x = 1, .y = 240,
      .width = XbyEighths(4) - 4,
      .height = 72,
      .align_x = AlignRight,
      .inset_y = 12,
      .inset_x = 16, // space for 5 digits
      .field = &tripTimeField,
      .font = &MEDIUM_NUMBERS_TEXT_FONT,
      .label_align_y = AlignTop,
      .border = BorderNone,
    },
    {
      .x = XbyEighths(4) + 1, .y = 240,
      .width = XbyEighths(4) - 4,
      .height = 72,
      .align_x = AlignRight,
      .inset_y = 12,
      .inset_x = 28, // space for 4 digits
      .field = &humanPowerField,
      .font = &MEDIUM_NUMBERS_TEXT_FONT,
      .label_align_y = AlignTop,
      .border = BorderNone,
    },
    {
      .x = 0, .y = 322,
      .width = XbyEighths(8),
      .height = 136,
      .field = &batteryVoltageGraph,
    },
    STATUS_BAR,
    {
      .field = NULL
    }
  }
};


// Screens in a loop, shown when the user short presses the power button
Screen *screens[] = { &mainScreen, &configScreen, NULL };


// Show our battery graphic
void battery_display() {
	static uint8_t oldsoc = 0xff;

	// Only trigger redraws if something changed
	if (l3_vars.volt_based_soc != oldsoc) {
		oldsoc = l3_vars.volt_based_soc;
		batteryField.dirty = true;
	}
}
