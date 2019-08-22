# SW102_Display

See this wiki pages with more information about this firmware:
- https://github.com/OpenSource-EBike-firmware/Color_LCD/wiki/Bafang-LCD-SW102
- https://github.com/OpenSource-EBike-firmware/SW102_LCD_Bluetooth/wiki

## How to build on Windows

TBD

## How to build on Linux

* Extract https://launchpad.net/gcc-arm-embedded/4.9/4.9-2015-q3-update/+download/gcc-arm-none-eabi-4_9-2015q3-20150921-linux.tar.bz2 into /usr/local/gcc-arm-none-eabi-4_9-2015q3.
* Run "make"

## Debugging bluetooth linux

Use https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Connect-for-desktop
Install this https://github.com/NordicSemiconductor/nrf-udev

Use this command to BLE update a target:
nrfutil dfu ble -ic NRF52 -p /dev/ttyACM0 --help

