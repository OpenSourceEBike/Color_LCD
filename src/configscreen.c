#include "screen.h"
#include "mainscreen.h"


static Field wheelMenus[] = {
    FIELD_EDITABLE_UINT("Max wheel speed", &l3_vars.ui8_wheel_max_speed, "km/h", 1, 99),
    FIELD_EDITABLE_UINT("Wheel perimeter", &l3_vars.ui16_wheel_perimeter, "mm", 750, 3000, .inc_step = 10),
    FIELD_EDITABLE_ENUM("Speed units", &l3_vars.ui8_units_type, "km/h", "mph"),
    FIELD_END
};

static Field batteryMenus[] = {
    FIELD_EDITABLE_UINT("Max current", &l3_vars.ui8_battery_max_current, "amps", 1, 30),
    FIELD_EDITABLE_UINT("Current ramp", &l3_vars.ui8_ramp_up_amps_per_second_x10, "amps", 4, 255, .div_digits = 1),
    // FIXME add remaining bat menu bindings
    FIELD_END
};

static Field topMenus[] = {
    FIELD_SCROLLABLE("Wheel settings", wheelMenus),
    FIELD_SCROLLABLE("Battery", batteryMenus),
    // FIXME and remaining top level menus
    FIELD_END
};

static Field configRoot = { .variant = FieldScrollable, .scrollable = { .entries = topMenus } };

//
// Screens
//
Screen configScreen = {
    {
        .color = ColorNormal,
        .field = &configRoot
    },
    {
        .field = NULL
    }
};
