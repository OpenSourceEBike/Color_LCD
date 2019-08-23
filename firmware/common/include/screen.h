#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "ugui.h"
#include "buttons.h"

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

#define MAX_FIELD_LEN 32

typedef enum {
  ColorNormal = 0, // white on black
  ColorInvert     // black on white
} ColorOp;

/**
 * specifies what type of data is in this field
 *
 * Note: might change someday to instead just be a pointer to a constant vtable like thing
 */
typedef enum {
  FieldDrawText = 0,
  FieldDrawTextPtr,
  FieldFill, // Fill with a solid color
  FieldMesh, // Fill with a mesh color
  FieldScrollable, // Contains a menu name and points to a submenu to optionally expand its place.  If at the root of a screen, submenu will be automatically expanded to fill remaining screen space
  FieldEditable, // An editable property with a human visible label and metadata for min/max/type of data and ptr to raw variable to render
  FieldCustom, // A field with a custom render function (provided by the user)
  FieldEnd // Marker record for the last entry in a scrollable submenu - never shown to user
} FieldVariant;

/**
 * What sorts of types are supported for FieldEditable.  Currently just uint8, uint16, bool and choosing a string from a list and storing its
 * index
 */
typedef enum {
  EditUInt = 0, // This is the default type if not specified
  EditEnum // Choose a string from a list
} EditableType;

struct FieldLayout; // Forward declaration

/**
 * Ready to render data (normally populated by comms code) which might be used on multiple different screens
 */
typedef struct Field {
  FieldVariant variant : 4;
  bool dirty : 1; // true if this data has changed and needs to be rerendered
  bool blink : 1; // if true, we should invoke the render function for this field every 500ms (or whatever the blink interval is) to possibly toggle animations on/off
  bool is_selected : 1; // if true this field is currently selected by the user (either in a scrollable or actively editing it)
  // bool is_rendered : 1; // true if we're showing this field on the current screen (if false, some fieldPrintf work can be avoided

  union {
    struct {
      char msg[MAX_FIELD_LEN];
    } drawText;

    struct {
      const char *msg; // A string stored in a ptr
    } drawTextPtr;

    struct {
      bool (*render)(struct FieldLayout *); // a custom render function, returns true if we did a render
    } custom;

    struct {
      struct Field *entries; // the menu entries for this submenu.
      const char *label; // the title shown in the GUI for this menu
      uint8_t first; // The first entry we are showing on the screen (ie for scrolling through a series of entries)
      uint8_t selected; // the currently highlighted entry
    } scrollable;

    struct {
      const char *label; // the label shown in the GUI for this item
      void *target; // the data we are showing/manipulating
      const EditableType typ;
      const uint8_t size : 3; // sizeof for the specified target - we support 1 or 2 or 4
      bool read_only : 1; // if true user can't really edit this

      // the following parameters are particular to the editable type
      union {

        struct {
          const char *units;
          const uint8_t div_digits: 4; // how many digits to divide by for fractions (i.e. 0 for integers, 1 for /10x, 2 for /100x, 3 /1000x
          const bool hide_fraction: 1; // if set, don't ever show the fractional part
          const uint32_t max_value, min_value; // min/max
          const uint32_t inc_step; // if zero, then 1 is assumed
        } number;

        struct {
          // we assume *target is a uint8_t
          const char **options; // An array of strings, with a NULL entry at the end to mark end of choices
        } editEnum;
      };
    } editable;
  };
} Field;

//
// Helper macros to declare fields more easily
//

#define FIELD_SCROLLABLE(lbl, arry) { .variant = FieldScrollable, .scrollable = { .label = lbl, .entries = arry } }

#define FIELD_EDITABLE_UINT(lbl, targ, unt, minv, maxv, ...) { .variant = FieldEditable, \
  .editable = { .typ = EditUInt, .label = lbl, .target = targ, .size = sizeof(*targ),  \
      .number = { .units = unt, .max_value = maxv, .min_value = minv, ##__VA_ARGS__ } } }

#define FIELD_READONLY_UINT(lbl, targ, unt, ...) { .variant = FieldEditable, \
  .editable = { .read_only = true, .typ = EditUInt, .label = lbl, .target = targ, .size = sizeof(*targ),  \
      .number = { .units = unt, ##__VA_ARGS__ } } }

// C99 allows anonymous constant arrays - take advantage of that here to make declaring the various options easy
#define FIELD_EDITABLE_ENUM(lbl, targ, ...) { .variant = FieldEditable, \
  .editable = { .typ = EditEnum, .label = lbl, .target = targ, .size = sizeof(EditableType), \
      .editEnum = { .options = (const char *[]){ __VA_ARGS__, NULL } } } }

#define FIELD_DRAWTEXT(...) { .variant = FieldDrawText, .drawText = { __VA_ARGS__  } }
#define FIELD_DRAWTEXTPTR(str, ...) { .variant = FieldDrawTextPtr, .drawTextPtr = { .msg = str, ##__VA_ARGS__  } }
#define FIELD_CUSTOM(cb) { .variant = FieldCustom, .custom = { .render = &cb  } }

#define FIELD_END { .variant = FieldEnd }


typedef int16_t Coord; // Change to int16_t for screens wider/longer than 128, screens shorter than 128 can use uint8_t

typedef enum {
  BorderNone = 0,
  BorderBottom = (1 << 0),
  BorderTop = (1 << 1),
  BorderLeft = (1 << 2),
  BorderRight = (1 << 3),
  BorderFat = (1 << 4), // two pixels tall
  BorderBox = BorderLeft | BorderRight | BorderTop | BorderBottom, // left,right,bottom,top
  // define others as needed
} BorderOp;


/// layouts can tell the field they are showing special rendering options
typedef enum {
  ModNone = 0,
  ModNoLabel = 1, // For editable fields: don't show label (normally), instead show just the data and the units
  ModLabelTop = 2, // For editable fields: show the label above the value, normally it is shown to the left
} LayoutModifier;

/**
 * Defines the layout of a field on a particular screen
 */
typedef struct FieldLayout {
  Coord x, y; // a y <0 means, start just below the previous lowest point on the screen, -1 is immediately below, -2 has one blank line, -3 etc...

  // for text fields if negative width is in # of characters. or 0 to determine length based on remaining screen width
  // For all other cases, width is in pixels
  Coord width;

  // for text fields use height = -1 to determine height based on font size.  for all fields 0 means 'rest of screen'
  // for other cases height is in pixels
  Coord height;

  BorderOp border; // an optional border to draw within this field

  ColorOp color : 4;
  LayoutModifier modifier : 4; // layouts can tell the field they are showing special rendering options

  Field *field; // The field to render in this location

  const UG_FONT *font; // If this field requires a font, use this.  Or if NULL auto select the biggest font that can hold the string

  uint32_t old_editable; // a cache value only used for editable fields, used to compare against previous values and redraw if needed.

} FieldLayout;


/** Called when a press has occured, return true if this function has handled the event (and therefore it should be cleared)
or false to let others handle it.

The order of handlers is:
* any active editable controls
* any scrollable controls on this screen
* the current screen handler
* the top level application handler
The top three handlers in the list above are handled by screen
*/
typedef bool (*ButtonEventHandler)(buttons_events_t events);

typedef struct {
  void (*onEnter)(); // If !NULL will be called whenever this screen is about to be shown (good to change globals etc)
  void (*onExit)(); // If !NULL will be called when this screen is no longer visible
  void (*onUpdate)(); // If !NULL, Called just before each update operation
  ButtonEventHandler onPress; // or NULL for no handler
  FieldLayout fields[];
} Screen;

// Standard vertical spacing for fonts
// #define FONT12_Y 14 // we want a little bit of extra space

void panicScreenShow(Screen *screen);
void screenShow(Screen *screen);
void screenUpdate();

/// Return the current visible screen
Screen *getCurrentScreen();

/// Returns true if the current screen handled the press
bool screenOnPress(buttons_events_t events);

void fieldPrintf(Field *field, const char *fmt, ...);

extern const UG_FONT *editable_label_font;
extern const UG_FONT *editable_value_font;
extern const UG_FONT *editable_units_font;

// The default is for editables to be two rows tall, with the data value on the second row
// define this as 1 if you want them to be one row tall (because you have a wide enough screen)
// #define EDITABLE_NUM_ROWS 2

// Used to define  positions in terms of # of 1/8ths of screen width/height (i.e. 4 is middle, 3 is slightly to left etc)
#define XbyEighths(n) ((SCREEN_WIDTH * (n)) / 8)
#define YbyEighths(n) ((SCREEN_HEIGHT * (n)) / 8)
#define Xby64(n) ((SCREEN_WIDTH * (n)) / 64)
#define Yby64(n) ((SCREEN_HEIGHT * (n)) / 64)

#ifdef SW102
#define SCREENFN_FORCE_LABELS buttons_get_m_state()

#define SCREENCLICK_START_EDIT M_CLICK
#define SCREENCLICK_STOP_EDIT M_CLICK
#define SCREEMCLICK_NEXT_SCREEN ONOFF_CLICK
#else
#define SCREENFN_FORCE_LABELS false

#define SCREENCLICK_START_EDIT ONOFF_CLICK
#define SCREENCLICK_STOP_EDIT UPDOWN_CLICK
#define SCREEMCLICK_NEXT_SCREEN UPDOWN_CLICK
#endif

