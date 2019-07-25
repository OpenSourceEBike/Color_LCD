#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "ugui.h"

/**
 * Main screen notes
 *
 * General approach:
 *
 * Make it look approximately like https://github.com/OpenSource-EBike-firmware/SW102_LCD_Bluetooth/blob/master/design/Bafang_SW102_Bluetooth_LCD-design-proposal_1.jpg
 *
 * Split UX logic from display layout with a small screen layer - to allow refactoring and sharing much of the 850C behavior/code
 * with the SW102.  Screen redraw time should go down dramatically because only change fields will need to be drawn.
 * The 850C (I think?) is calling that redraw function a lot - a full screen transfer to the display for each field update.
 *
 * initially just show power remaining in pack, SOC, time, amount of assist
 * make assist go up/down with button press. (initially copy 850C button code, but _real soon_ to allow sharing the C source for button/packet/UX handling between both
 * modern displays)
 *
 * remove call to UG_SetRefresh, it is very expensive.  Much better to just call lcd_refresh() from screenUpdate()
 *
 * define a screen as:
 *
 * an array of FieldLayouts.  each FieldLayout is a (typically constant) struct:
 * xpos, ypos, width, height, colorOpt, fieldPtr
 * colorOpt is an enum of ColorNormal, ColorInvert
 * eventually: ColorCritical, ColorBlink (for use in menu selections) -for color
 * LCDs these enums could map to the theme picked for the UX, for black and white LCDs they map to black or white
 *
 * end of screen is market by a FieldLayout with a null field member
 *
 * fieldPtr points to a particular field with ready to render data (and fields might be shared by multiple screens).
 * Each Field contains
 *   variantCode, isDirty and extra data appropriate for that variant
 *
 * isDirty is true if the field needs to be redrawn in the gui (because data has changed)
 * datavariant is a union that depends on the opcode:
 *
 * drawText: font ptr, char msg[MAXSTRLEN]
 * fillBox: nothing - just fills box based on fore/back color
 * drawBat: soc - draw a bat icon with SOC
 * drawPlot: maxval - max value seen, points - an array of 64 previous data values, to be drawn as a plot, one per column

 *
 * helper functions:
 * fieldPrintf(fieldptr, "str %d", 5) - sets the string for the specified fields, marks the field as dirty if the string changed
 * fieldSetSOC(fieldptr, 32) - sets state of charge and marks field as dirty if the soc changed
 * fieldAddPlot(fieldptr, value) - add a new data point to a plot field
 *
 * When new state is received from the controller, fieldX...() will be called to mark the various fields as dirty.  These functions
 * are cheap and can be called when each rx packet is parsed.  If any field changed in a user visible way the field will be internally
 * marked as dirty and later updateScreen() will show that new value
 *
 * screenShow(screenptr) - set the current screen
 * screenUpdate() - redraw the minimum set of dirty fields (or the whole screen if the screen has changed).
 *   if any fields are blinking the blink animation will be serviced here as well.
 *
 * NOTE: this approach could be extended to include nice support for showing vertically scrolling menus.  Initial version
 * won't do this but should make the config screen fairly easy to code up.
 *
 * add a datavariant for
 * drawMenu: curSelection, const char **menuOptions, const char **menuValues, onChange(tbd) - properly handle showing a scrolling menu
 *   that might be logically longer than our screen. onChange callback will be called when the user changes menu entries
 *
 * bool screenHandlePress(buttonEnum)
 * if a screen is showing a menu field, it might need to intercept and handle button presses. Call this function from
 * the main loop when a press occurs.  This function will return true if it has handled this press event (and therefore
 * you should not do anything else with it).
 *
 */

#define MAX_FIELD_LEN 16

typedef enum {
  ColorNormal = 0, // white on black
  ColorInvert  // black on white
} ColorOp;

/**
 * specifies what type of data is in this field
 *
 * Note: might change someday to instead just be a pointer to a constant vtable like thing
 */
typedef enum {
  FieldDrawText = 0,
  FieldFill, // Fill with a solid color
  FieldMesh, // Fill with a mesh color
} FieldVariant;

/**
 * Ready to render data (normally populated by comms code) which might be used on multiple different screens
 */
typedef struct {
  FieldVariant variant; //
  bool dirty; // true if this data has changed and needs to be rerendered

  union {
    struct {
      const UG_FONT *font;
      char msg[MAX_FIELD_LEN];
    } drawText;
  };
} Field;

typedef int8_t Coord; // Change to int16_t for screens wider/longer than 128

/**
 * Defines the layout of a field on a particular screen
 */
typedef struct {
  const Coord x, y;
  const Coord width; // for text fields width is in # of characters, or -1 to determine length based on strlen
  const Coord height; // for text fields use height = -1 to determine height based on font size

  const ColorOp color;
  Field *field;
} FieldLayout;

typedef const FieldLayout Screen[];


void screenShow(const Screen *screen);
void screenUpdate();
void fieldPrintf(Field *field, const char *fmt, ...);
