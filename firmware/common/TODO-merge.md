## Easy merges (sequence, listing source files):

* eeprom
* rtc

## Mostly done

* buttons - test in sw102

## done

* utils
* remove UG_PutString_with_length copypasta
* ugui
* fonts BITS_FONT_61X99 BITS_FONT_45X72
* state
* uart
* fault
* screen

## Hard merges:

* configscreen
* mainscreen (excluding graphs)
* adc
* boot screen
* add graphs
* final layout for mainscreen

## other work items:

* implement C_TRANSPARENT on 850C
* use colors in the 850C gui
* add instrumentation to measure times of key operations (main loop, frame update etc)
* check that linker is stripping all unused functions (for both platforms)
* dramatically shrink the buttons.c code
* remove ui8_***_imperial
* make trip distance work with imperial again (see FIXME in lcd.c)
* uart_get_rx_buffer_rdy in the 850C might be a little bit race conditiony - eval and fix
* change lcd brightenss eeprom value to a pctage in the 850C
* implement fault.c for 850C
* 850C doesn't seem to do proper dependency checking - changes to .h files should cause new builds
