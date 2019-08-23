## Remaining merge tasks:

* fix configscreen to be prettier (use color backgrounds, show units, bigger font, use color arrow for selection)
* fix trip time to use a readonly fieldtype (so it lays out correctly on mainscreen)
* labels should be center aligned
* add graphs
* make local ADC report voltage, so bootscreen/sim-motor can check it.
* fix boot screen layout to be prettier - add URL to github project
* make trip distance work with imperial again (see FIXME in lcd.c) - applies to all imperial data
* use a slightly smaller font for the digits after the dot 45x72, big is 61x99. apply to speed field

## other work items:
* changing # of digits drawn forces a full redraw of the field, which produces a very slight visible flash as the label is erased and redrawn
* delay_ms(120) seems to wait a lot longer than 120ms
* uart_get_rx_buffer_rdy in the 850C might be a little bit race conditiony - eval and fix
* change lcd brightenss eeprom value to a pctage in the 850C
*     set_lcd_backlight(); // fix backlight levels - FIXME, I'm calling this from interrupt context here which is probably ungood
* tell others to gcc 7.3.1 (and make a develop.md or something to tell others, also mention use eclipse formatting rules) - https://github.com/gnu-mcu-eclipse/arm-none-eabi-gcc/releases/tag/v7.3.1-1.1
* make most usages use renderDrawPtrText

## Mostly done

* still need to change eeprom.c (change most of 850C version to be a HAL similar to eeprom-hw.c)

## done

* utils
* remove UG_PutString_with_length copypasta
* ugui
* fonts BITS_FONT_61X99 BITS_FONT_45X72
* state
* uart
* fault
* screen
* implement C_TRANSPARENT on 850C
* rtc
* buttons - test in sw102
* mainscreen (excluding graphs)
* configscreen
* add screen looping
* make sim motor work (based on adc voltage detection)
* add fault screen 
* fix button press handling
* not properly blinking the edit cursor - getting covered by item beneath?
* we are redrawing menus when we should not (due to blink event handling probably?)
* we are redrawing speed when we should not (also, don't do the fillrect and borders unless we are from scratch, instead don't use c_transparent in the font)
* show labels properly on hte 850C layouts
* more implement fault.c for 850C 
* add a hard fault handler to catch and print bad ptr accesses
* use the slick 850C battery graphic case as a new renderBattery function
* show a wider bar for BorderFat - scale based on screen height (to make config screen look better)

## todo later 

* per discussion with @casainho no problem - "add motor RPM, ERPS and PWM duty cycle to the secondary info page"
* stop using ui32_g_layer_2_can_execute for mutual exclusion with the ISR
* add instrumentation to measure times of key operations (main loop, frame update etc)
* check that linker is stripping all unused functions (for both platforms)
* dramatically shrink the buttons.c code
* remove ui8_***_imperial (by making auto scaling smarter in render)
* 850C doesn't seem to do proper dependency checking - changes to .h files should cause new builds
* Per @lowPerformer: We can also extend the fonts by our "special" characters we need, f.i. ASCII ':' can be a 'W'. I did that with MY_FONT_8X12 where 0x1F is a '°' like in °C.

