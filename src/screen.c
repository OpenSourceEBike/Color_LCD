/*
 Screen layer for ugui

 Copyright 2019, S. Kevin Hester, kevinh@geeksville.com

 (MIT License)
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial
 portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "screen.h"
#include "lcd.h"
#include "ugui.h"

extern UG_GUI gui;

static UG_COLOR getBackColor(const FieldLayout *layout)
{
  switch (layout->color)
  {
  case ColorInvert:
    return C_WHITE;

  case ColorNormal:
  default:
    return C_BLACK;
  }
}

static UG_COLOR getForeColor(const FieldLayout *layout)
{
  switch (layout->color)
  {
  case ColorInvert:
    return C_BLACK;

  case ColorNormal:
  default:
    return C_WHITE;
  }
}

// Returns true if we decided to draw something
typedef bool (*FieldRenderFn)(FieldLayout *layout);

static const FieldRenderFn renderers[];

/// If true blink changed to be true or false this tick and we should redraw anything that is animated
static bool blinkChanged;
static bool blinkOn;

static bool renderDrawText(FieldLayout *layout)
{
  Field *field = layout->field;

  // Allow developer to use this shorthand for one row high text fields
  if (layout->height == -1)
    layout->height = field->drawText.font->char_height;

  UG_S16 width =
      (layout->width < 0) ?
          -layout->width * (field->drawText.font->char_width + gui.char_h_space) : layout->width;
  UG_S16 height = layout->height;

  UG_FontSelect(field->drawText.font);
  UG_COLOR back = getBackColor(layout);
  UG_SetBackcolor(back);
  UG_SetForecolor(getForeColor(layout));

  // ug fonts include no blank space at the beginning, so we always include one col of padding
  UG_FillFrame(layout->x, layout->y, layout->x + width - 1,
      layout->y + height - 1, back);
  UG_PutString(layout->x + 1, layout->y, field->drawText.msg);
  return true;
}

static bool renderFill(FieldLayout *layout)
{
  assert(layout->width >= 1);
  assert(layout->height >= 1);

  UG_FillFrame(layout->x, layout->y, layout->x + layout->width - 1,
      layout->y + layout->height - 1, getForeColor(layout));
  return true;
}

static bool renderMesh(FieldLayout *layout)
{
  assert(layout->width >= 1);
  assert(layout->height >= 1);

  UG_DrawMesh(layout->x, layout->y, layout->x + layout->width - 1,
      layout->y + layout->height - 1, getForeColor(layout));
  return true;
}

/**
 * If we are selected, highlight this item with a bar to the left (on color screens possibly draw a small
 * color pointer or at least color the line something nice.
 */
static void drawSelectionMarker(FieldLayout *layout)
{
  if(layout->field && layout->field->is_selected) // Only consider doing this on items that might be animated
    UG_DrawLine(layout->x, layout->y, layout->x, layout->y + layout->height - 1, blinkOn ? getForeColor(layout) : getBackColor(layout));
}

#define MAX_SCROLLABLE_ROWS 4 // Max number of rows we can show on one screen (including header)

const Coord screenWidth = 64, screenHeight = 128; // FIXME, for larger devices allow screen objcts to nest inside other screens

const bool renderLayouts(FieldLayout *layouts, bool forceRender)
{
  bool didDraw = false; // we only render to hardware if something changed

  // For each field if that field is dirty (or the screen is) redraw it
  for (FieldLayout *layout = layouts; layout->field; layout++)
  {
    // We always render dirty items, or items that might need to show blink animations
    if (layout->field->dirty || (layout->field->blink && blinkChanged) || forceRender)
    {
      if (layout->width == 0)
        layout->width = screenWidth - layout->x;

      if (layout->height == 0)
        layout->height = screenHeight - layout->y;

      didDraw |= renderers[layout->field->variant](layout);

      drawSelectionMarker(layout);
    }
  }

  // We clear the dirty bits in a separate pass because multiple layouts on the screen might share the same field
  for (const FieldLayout *layout = layouts; layout->field; layout++)
  {
    layout->field->dirty = false;
  }

  return didDraw;
}

// If true, the scroll position changed and force a complete redraw
// FIXME - heading shouldn't be redrawn
// FIXME - currently limited to one scrollable per screen
static bool forceScrollableRelayout;

// If the user is editing an editable, this will be it
static Field *curActiveEditable = NULL;

#define MAX_SCROLLABLE_DEPTH 3 // How deep can we nest scrollables in our stack

static Field *scrollableStack[MAX_SCROLLABLE_DEPTH];
int scrollableStackPtr = 0; // Points to where to push the next entry (so if zero, stack is empty)

// Return the scrollable we are currently showing the user, or NULL if none
// The (currently only one allowed per screen) scrollable that is currently being shown to the user.
// if the scrollable changes, we'll need to regenerate the entire render
static Field* getActiveScrollable()
{
  return scrollableStackPtr ? scrollableStack[scrollableStackPtr - 1] : NULL;
}

/**
 * The user just clicked on a scrollable entry, descend down into it
 */
static void enterScrollable(Field *f)
{
  assert(scrollableStackPtr < MAX_SCROLLABLE_DEPTH);
  scrollableStack[scrollableStackPtr++] = f;

  // We always set blink for scrollables, because they contain child items that might need to blink
  f->blink = true;

  // NOTE: Only the root scrollable is ever checked for 'dirty' by the main screen renderer,
  // so that's the one we set
  scrollableStack[0]->dirty = true;

  forceScrollableRelayout = true;
}

/**
 * The user just clicked to exit a scrollable entry, ascend to the entry above us or if we are the top
 * go back to the main screen
 */
static void exitScrollable()
{
  assert(scrollableStackPtr > 0);
  scrollableStackPtr--;

  Field *f = getActiveScrollable();
  if (f)
  {
    // Parent was a scrollable, show it
    f->dirty = true;
    forceScrollableRelayout = true;
  }
  else
  {
    // otherwise we just leave the screen showing the top scrollable
  }
}

static bool renderActiveScrollable(FieldLayout *layout, Field *field)
{
  const Coord rowHeight = 32; // 3 data rows 32 pixels tall + one 32 pixel header

  static FieldLayout rows[MAX_SCROLLABLE_ROWS + 1]; // Used to layout each of the currently visible rows + heading

  Field *scrollable = getActiveScrollable();
  bool weAreExpanded = scrollable == field;

  // If we are expanded show our heading and the current visible child elements
  // Otherwise just show our label so that the user might select us to expand
  if (weAreExpanded)
  {
    // FIXME - we shouldn't need to relayout scrollables on blink transition, but currently we share the static rows[] array when we really should not, because
    // it is used _both_ in the drawing of expanded elements and the non expanded case.  This problem becomes apparant if you remove this blinkChanged check.
    if (forceScrollableRelayout || blinkChanged)
    {
      static Field blankRows[MAX_SCROLLABLE_ROWS]; // Used to fill with blank space if necessary
      static Field heading = FIELD_DRAWTEXT(&FONT_5X12);

      bool hasMoreRows = true; // Once we reach an invalid row we stop rendering and instead fill with blank space

      forceScrollableRelayout = false;
      for (int i = 0; i < MAX_SCROLLABLE_ROWS; i++)
      {
        FieldLayout *r = rows + i;

        r->x = layout->x;
        r->y = layout->y + rowHeight * i;
        r->width = layout->width;
        r->height = rowHeight;

        if (i == 0)
        { // heading
          fieldPrintf(&heading, "%s", field->scrollable.label);
          r->field = &heading;
          r->color = ColorInvert;
        }
        else
        {
          // visible menu rows, starting with where the user has scrolled to
          const int entryNum = field->scrollable.first + i - 1;
          Field *entry = &field->scrollable.entries[entryNum];

          entry->dirty = true; // Force it to be redrawn
          if (entry->variant == FieldEnd)
            hasMoreRows = false;

          // if the current row is valid, render that, otherwise render blank space
          if (hasMoreRows)
          {
            r->field = entry;
            entry->is_selected = (entryNum == field->scrollable.selected);
          }
          else
          {
            r->field = &blankRows[i];
            r->field->variant = FieldFill;
            r->color = ColorInvert; // black box for empty slots at end
          }

          r->field->dirty = true; // Force rerender
        }

        rows[MAX_SCROLLABLE_ROWS].field = NULL; // mark end of array (for rendering)
      }
    }
  }
  else
  {
    // Just draw our label (not highlighted) - show selection bar if necessary
    FieldLayout *r = &rows[0];

    r->x = layout->x;
    r->y = layout->y;
    r->width = layout->width;
    r->height = layout->height;

    static Field label = FIELD_DRAWTEXT(&FONT_5X12);
    fieldPrintf(&label, "%s", field->scrollable.label);
    r->field = &label;
    r->color = ColorNormal;

    // If we are inside a scrollable and selected, blink
    if(scrollable)
      label.is_selected = field == &scrollable->scrollable.entries[scrollable->scrollable.selected];
    else
      label.is_selected = false;

    rows[1].field = NULL; // mark end of array (for rendering)
  }

  // draw (or redraw if necessary) our current set of visible rows
  return renderLayouts(rows, false);
}

static bool renderScrollable(FieldLayout *layout)
{
  if (!getActiveScrollable()) // we are the first scrollable on this screen, use us to init the stack
    enterScrollable(layout->field);

  // If we are being asked to render the root scrollable, instead we want to substitute the deepest scrollable
  // in the stack
  Field *field = layout->field;
  if (scrollableStack[0] == field)
    field = getActiveScrollable();

  return renderActiveScrollable(layout, field);
}

// Get the numeric value of an editable number, properly handling different possible byte encodings
static int32_t getEditableNumber(Field *field)
{
  switch (field->editable.size)
  {
  case 1:
    return *(uint8_t*) field->editable.target;
  case 2:
    return *(int16_t*) field->editable.target;
  case 4:
    return *(int32_t*) field->editable.target;
  default:
    assert(0);
    return 0;
  }
}

// Set the numeric value of an editable number, properly handling different possible byte encodings
static void setEditableNumber(Field *field, uint32_t v)
{
  switch (field->editable.size)
  {
  case 1:
    *(uint8_t*) field->editable.target = (uint8_t) v;
    break;
  case 2:
    *(uint16_t*) field->editable.target = (uint16_t) v;
    break;
  case 4:
    *(uint32_t*) field->editable.target = (uint32_t) v;
    break;
  default:
    assert(0);
  }
}

static bool renderEditable(FieldLayout *layout)
{
  Field *field = layout->field;
  UG_S16 width = layout->width;
  UG_S16 height = layout->height;

  const UG_FONT *font = &FONT_5X12;
  UG_FontSelect(font);
  UG_COLOR back = getBackColor(layout), fore = getForeColor(layout);
  UG_SetBackcolor(back);
  UG_SetForecolor(fore);

  // ug fonts include no blank space at the beginning, so we always include one col of padding
  UG_FillFrame(layout->x, layout->y, layout->x + width - 1,
      layout->y + height - 1, back);

  UG_PutString(layout->x + 1, layout->y, (char*) field->editable.label);

  // draw editable value
  char msgbuf[MAX_FIELD_LEN];
  const char *msg;
  uint32_t num = getEditableNumber(field);
  switch (field->editable.typ)
  {
  case EditUInt:
    // FIXME properly handle div_digits
    snprintf(msgbuf, sizeof(msgbuf), "%lu", num);
    msg = msgbuf;
    break;
  case EditEnum:
    msg = field->editable.editEnum.options[num];
    break;
  default:
    assert(0);
    break;
  }

  // Print the value (inverted if we are editing it)
  if (curActiveEditable == field)
  {
    UG_SetBackcolor(fore);
    UG_SetForecolor(back);
  }

  // right justify value on the second line
  UG_PutString(layout->x + width - strlen(msg) * (font->char_width + gui.char_h_space),
      layout->y + FONT12_Y, (char*) msg);

  return true;
}

static bool renderEnd(FieldLayout *layout)
{
  assert(0); // This should never be called I think
  return true;
}

static void forceScrollableRender()
{
  Field *active = getActiveScrollable();
  assert(active);
  scrollableStack[0]->dirty = true; // the gui thread only looks in the root scrollable to find dirty
  forceScrollableRelayout = true;
}

static int countEnumOptions(Field *s)
{
  const char **e = s->editable.editEnum.options;

  int n = 0;
  while (*e++)
    n++;

  return n;
}

/**
 * increment/decrement an editable
 */
static void changeEditable(bool increment)
{
  Field *f = curActiveEditable;
  assert(f);

  int v = getEditableNumber(f);

  switch (f->editable.typ)
  {
  case EditUInt:
  {
    int step = f->editable.number.inc_step;

    if (step == 0)
      step = 1;

    v += step * (increment ? 1 : -1);
    if (v < f->editable.number.min_value) // loop around
      v = f->editable.number.max_value;
    else if (v > f->editable.number.max_value)
      v = f->editable.number.min_value;
    setEditableNumber(f, v);
    break;
  }
  case EditEnum:
  {
    int numOpts = countEnumOptions(f);
    v += increment ? 1 : -1;
    if (v < 0) // loop around
      v = numOpts - 1;
    else if (v >= numOpts)
      v = 0;
    setEditableNumber(f, v);
    break;
  }
  default:
    assert(0);
    break;
  }
}

// Returns true if we've handled the event (and therefore it should be cleared)
static bool onPressEditable(buttons_events_t events)
{
  bool handled = false;
  Field *s = curActiveEditable;

  if (events & UP_CLICK)
  {
    changeEditable(true);
    handled = true;
  }

  if (events & DOWN_CLICK)
  {
    changeEditable(false);
    handled = true;
  }

  // Mark that we are no longer editing - click pwr button to exit
  if (events & ONOFF_CLICK)
  {
    curActiveEditable = NULL;

    handled = true;
  }

  if(handled) {
    s->dirty = true; // redraw our position

    // If we are inside a scrollable, tell the GUI that scrollable also needs to be redrawn
    Field *scrollable = getActiveScrollable();
    if (scrollable)
    {
      scrollableStack[0]->dirty = true; // we just changed something, make sure we get a chance to be redrawn
    }
  }

  return handled;
}

int countEntries(Field *s)
{
  Field *e = s->scrollable.entries;

  int n = 0;
  while (e && e->variant != FieldEnd)
  {
    n++;
    e++;
  }

  return n;
}

// Returns true if we've handled the event (and therefore it should be cleared)
// if first or selected changed, mark our scrollable as dirty (so child editables can be drawn)
static bool onPressScrollable(buttons_events_t events)
{
  bool handled = false;
  Field *s = getActiveScrollable();

  if (!s)
    return false; // no scrollable is active

  if (events & UP_CLICK)
  {
    if (s->scrollable.selected >= 1)
    {
      s->scrollable.selected--;
    }

    if (s->scrollable.selected < s->scrollable.first) // we need to scroll the whole list up some
      s->scrollable.first = s->scrollable.selected;

    forceScrollableRender();
    ;
    handled = true;
  }

  if (events & DOWN_CLICK)
  {
    int numEntries = countEntries(s);

    if (s->scrollable.selected < numEntries - 1)
    {
      s->scrollable.selected++;
    }

    int numDataRows = MAX_SCROLLABLE_ROWS - 1;
    int lastVisibleRow = s->scrollable.first + numDataRows - 1;
    if (s->scrollable.selected > lastVisibleRow) // we need to scroll the whole list down some
      s->scrollable.first = s->scrollable.selected - numDataRows + 1;

    forceScrollableRender();
    handled = true;
  }

  // If we aren't already editing anything, start now (note: we will only be called if some active editable
  // hasn't already handled this button
  if (events & M_CLICK)
  {
    Field *clicked = &s->scrollable.entries[s->scrollable.selected];

    switch (clicked->variant)
    {
    case FieldEditable:
      curActiveEditable = clicked;
      curActiveEditable->dirty = true; // force redraw with highlighting
      handled = true;
      forceScrollableRender(); // FIXME, I'm not sure if this is really required
      break;

    case FieldScrollable:
      enterScrollable(clicked);
      handled = true;
      break;

    default:
      break;
    }
  }

  // click power button to exit out of menus
  if (events & ONOFF_CLICK)
  {
    exitScrollable();
  }

  return handled;
}

/**
 * Used to map from FieldVariant enums to rendering functions
 */
static const FieldRenderFn renderers[] = { renderDrawText, renderFill,
    renderMesh, renderScrollable, renderEditable, renderEnd };

static Screen *curScreen;
static bool screenDirty;

bool screenOnPress(buttons_events_t events)
{
  bool handled = false;

  if (curActiveEditable)
    handled |= onPressEditable(events);

  if (!handled)
    handled |= onPressScrollable(events);

  if (!handled && curScreen && curScreen->onPress)
    handled |= curScreen->onPress(events);

  return handled;
}

// A low level screen render that doesn't use soft device or call exit handlers (useful for the critical fault handler ONLY)
void panicScreenShow(Screen *screen)
{
  curActiveEditable = NULL;
  scrollableStackPtr = 0; // new screen might not have one, we will find out when we render
  curScreen = screen;
  screenDirty = true;
  screenUpdate(); // Force a draw immediately
}


void screenShow(Screen *screen)
{
  if (curScreen && curScreen->onExit)
    curScreen->onExit();

  panicScreenShow(screen);
}

void screenUpdate()
{
  if (!curScreen)
    return;

  bool didDraw = false; // we only render to hardware if something changed

  // Every 200ms toggle any blinking animations
  static uint8_t blinkCounter;
  blinkCounter = (blinkCounter + 1) % 10;
  blinkChanged = (blinkCounter == 0);
  if(blinkChanged) {
    blinkOn = !blinkOn;
  }

  if (screenDirty)
  {
    ; // clear screen (to prevent turds from old screen staying around)
    UG_FillScreen(C_BLACK);
    didDraw = true;
  }

  // For each field if that field is dirty (or the screen is) redraw it
  didDraw |= renderLayouts(curScreen->fields, screenDirty);

  // flush the screen to the hardware
  if (didDraw)
  {
    lcd_refresh();
  }

  screenDirty = false;
}

void fieldPrintf(Field *field, const char *fmt, ...)
{
  va_list argp;
  va_start(argp, fmt);
  char buf[sizeof(field->drawText.msg)] = "";
  vsnprintf(buf, sizeof(buf), fmt, argp);
  if (strcmp(buf, field->drawText.msg) != 0)
  {
    strcpy(field->drawText.msg, buf);
    field->dirty = true;
  }

  va_end(argp);
}

