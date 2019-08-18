## Hard merges:

* make sim motor work
* mainscreen (excluding graphs)
* final layout for mainscreen
* configscreen
* use the 850C battery graphic case as a new renderBattery function
* adc
* boot screen
* add graphs
* add fault screen
* use a slightly smaller font for the digits after the dot

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


## other work items:

* use colors in the 850C gui
* make trip distance work with imperial again (see FIXME in lcd.c)
* uart_get_rx_buffer_rdy in the 850C might be a little bit race conditiony - eval and fix
* change lcd brightenss eeprom value to a pctage in the 850C

## todo later 

* add instrumentation to measure times of key operations (main loop, frame update etc)
* check that linker is stripping all unused functions (for both platforms)
* dramatically shrink the buttons.c code
* remove ui8_***_imperial (by making auto scaling smarter in render)
* implement fault.c for 850C
* 850C doesn't seem to do proper dependency checking - changes to .h files should cause new builds

