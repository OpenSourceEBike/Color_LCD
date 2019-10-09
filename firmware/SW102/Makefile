PROJECT_NAME     := nrf51822_sw102
TARGETS          := nrf51822_sw102
OUTPUT_DIRECTORY := _build

# Only this directory contains bins that should be uploaded to github for end-user-use
RELEASE_DIRECTORY := _release

SDK_ROOT := ./nRF5_SDK_12.3.0
PROJ_DIR := .

$(OUTPUT_DIRECTORY)/nrf51822_sw102.out: \
  LINKER_SCRIPT  := gcc_nrf51.ld

# dev platform specific.
# OPENOCD_PATH := E:/nrf5/Toolchain/OpenOCD/0.10.0-12-20190422-2015/bin
# OPENOCD_BIN := openocd.exe
# NRFUTIL := $(SDK_ROOT)/external_tools/Windows/nrfutil.exe

OPENOCD_PATH := /usr/local/share/openocd/bin
OPENOCD_BIN := openocd
NRFUTIL := nrfutil

OPENOCD := '$(OPENOCD_PATH)/$(OPENOCD_BIN)' -f $(OPENOCD_PATH)/../scripts/interface/stlink.cfg -f $(OPENOCD_PATH)/../scripts/target/nrf51.cfg

COMMON_DIR := ../common

# Source files common to all targets
SRC_FILES += \
  $(PROJ_DIR)/src/sw102/main.c \
  $(PROJ_DIR)/src/sw102/lcd.c \
  $(PROJ_DIR)/src/sw102/button.c \
  $(PROJ_DIR)/src/sw102/ble_services.c \
  $(PROJ_DIR)/src/sw102/adc.c \
  $(PROJ_DIR)/src/sw102/eeprom_hw.c \
  $(PROJ_DIR)/src/sw102/rtc.c \
  $(PROJ_DIR)/src/sw102/mainscreen-sw102.c \
  $(COMMON_DIR)/src/ugui.c \
  $(COMMON_DIR)/src/fault.c \
  $(COMMON_DIR)/src/buttons.c \
  $(PROJ_DIR)/src/sw102/uart.c \
  $(COMMON_DIR)/src/utils.c \
  $(COMMON_DIR)/src/state.c \
  $(COMMON_DIR)/src/eeprom.c \
  $(COMMON_DIR)/src/screen.c \
  $(COMMON_DIR)/src/fonts.c \
  $(COMMON_DIR)/src/mainscreen.c \
  $(COMMON_DIR)/src/configscreen.c \
  $(SDK_ROOT)/components/libraries/util/app_error.c \
  $(SDK_ROOT)/components/libraries/util/app_error_weak.c \
  $(SDK_ROOT)/components/libraries/util/nrf_assert.c \
  $(SDK_ROOT)/components/libraries/timer/app_timer.c \
  $(SDK_ROOT)/components/libraries/util/app_util_platform.c \
  $(SDK_ROOT)/components/libraries/util/sdk_mapped_flags.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu_settings.c \
  $(SDK_ROOT)/components/libraries/fstorage/fstorage.c \
  $(SDK_ROOT)/components/libraries/fds/fds.c \
  $(SDK_ROOT)/components/drivers_nrf/adc/nrf_drv_adc.c \
  $(SDK_ROOT)/components/drivers_nrf/common/nrf_drv_common.c \
  $(SDK_ROOT)/components/drivers_nrf/clock/nrf_drv_clock.c \
  $(SDK_ROOT)/components/drivers_nrf/gpiote/nrf_drv_gpiote.c \
  $(SDK_ROOT)/components/drivers_nrf/spi_master/nrf_drv_spi.c \
  $(SDK_ROOT)/components/drivers_nrf/uart/nrf_drv_uart.c \
  $(SDK_ROOT)/components/drivers_nrf/wdt/nrf_drv_wdt.c \
  $(SDK_ROOT)/components/ble/common/ble_advdata.c \
  $(SDK_ROOT)/components/ble/ble_advertising/ble_advertising.c \
  $(SDK_ROOT)/components/ble/common/ble_conn_params.c \
  $(SDK_ROOT)/components/ble/common/ble_conn_state.c \
  $(SDK_ROOT)/components/ble/ble_services/ble_nus/ble_nus.c \
  $(SDK_ROOT)/components/ble/ble_services/ble_bas/ble_bas.c \
  $(SDK_ROOT)/components/ble/ble_services/ble_dis/ble_dis.c \
  $(SDK_ROOT)/components/ble/ble_services/ble_cscs/ble_cscs.c \
  $(SDK_ROOT)/components/ble/ble_services/ble_cscs/ble_sc_ctrlpt.c \
  $(SDK_ROOT)/components/ble/common/ble_srv_common.c \
  $(SDK_ROOT)/components/ble/peer_manager/gatt_cache_manager.c \
  $(SDK_ROOT)/components/ble/peer_manager/gatts_cache_manager.c \
  $(SDK_ROOT)/components/ble/peer_manager/id_manager.c \
  $(SDK_ROOT)/components/ble/peer_manager/peer_data.c \
  $(SDK_ROOT)/components/ble/peer_manager/peer_data_storage.c \
  $(SDK_ROOT)/components/ble/peer_manager/peer_database.c \
  $(SDK_ROOT)/components/ble/peer_manager/peer_id.c \
  $(SDK_ROOT)/components/ble/peer_manager/peer_manager.c \
  $(SDK_ROOT)/components/ble/peer_manager/pm_buffer.c \
  $(SDK_ROOT)/components/ble/peer_manager/pm_mutex.c \
  $(SDK_ROOT)/components/ble/peer_manager/security_dispatcher.c \
  $(SDK_ROOT)/components/ble/peer_manager/security_manager.c \
  $(SDK_ROOT)/components/toolchain/gcc/gcc_startup_nrf51.S \
  $(SDK_ROOT)/components/toolchain/system_nrf51.c \
  $(SDK_ROOT)/components/softdevice/common/softdevice_handler/softdevice_handler.c \

# Include folders common to all targets
INC_FOLDERS += \
  $(PROJ_DIR)/include \
  $(COMMON_DIR)/include \
  $(SDK_ROOT)/components \
  $(SDK_ROOT)/components/boards \
  $(SDK_ROOT)/components/device \
  $(SDK_ROOT)/components/softdevice/common/softdevice_handler \
  $(SDK_ROOT)/components/softdevice/s130/headers \
  $(SDK_ROOT)/components/softdevice/s130/headers/nrf51 \
  $(SDK_ROOT)/components/drivers_nrf/adc \
  $(SDK_ROOT)/components/drivers_nrf/ble_flash \
  $(SDK_ROOT)/components/drivers_nrf/clock \
  $(SDK_ROOT)/components/drivers_nrf/common \
  $(SDK_ROOT)/components/drivers_nrf/delay \
  $(SDK_ROOT)/components/drivers_nrf/gpiote \
  $(SDK_ROOT)/components/drivers_nrf/hal \
  $(SDK_ROOT)/components/drivers_nrf/lpcomp \
  $(SDK_ROOT)/components/drivers_nrf/nrf_soc_nosd \
  $(SDK_ROOT)/components/drivers_nrf/pdm \
  $(SDK_ROOT)/components/drivers_nrf/power \
  $(SDK_ROOT)/components/drivers_nrf/ppi \
  $(SDK_ROOT)/components/drivers_nrf/qdec \
  $(SDK_ROOT)/components/drivers_nrf/radio_config \
  $(SDK_ROOT)/components/drivers_nrf/rng \
  $(SDK_ROOT)/components/drivers_nrf/rtc \
  $(SDK_ROOT)/components/drivers_nrf/sdio \
  $(SDK_ROOT)/components/drivers_nrf/spi_master \
  $(SDK_ROOT)/components/drivers_nrf/spi_slave \
  $(SDK_ROOT)/components/drivers_nrf/swi \
  $(SDK_ROOT)/components/drivers_nrf/timer \
  $(SDK_ROOT)/components/drivers_nrf/twi_master \
  $(SDK_ROOT)/components/drivers_nrf/twis_slave \
  $(SDK_ROOT)/components/drivers_nrf/uart \
  $(SDK_ROOT)/components/drivers_nrf/wdt \
  $(SDK_ROOT)/components/ble/ble_advertising \
  $(SDK_ROOT)/components/ble/ble_services/ble_ancs_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_ans_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_bas \
  $(SDK_ROOT)/components/ble/ble_services/ble_bas_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_cscs \
  $(SDK_ROOT)/components/ble/ble_services/ble_cts_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_dfu \
  $(SDK_ROOT)/components/ble/ble_services/ble_dis \
  $(SDK_ROOT)/components/ble/ble_services/ble_gls \
  $(SDK_ROOT)/components/ble/ble_services/ble_hids \
  $(SDK_ROOT)/components/ble/ble_services/ble_hrs_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_hrs \
  $(SDK_ROOT)/components/ble/ble_services/ble_hts \
  $(SDK_ROOT)/components/ble/ble_services/ble_ias \
  $(SDK_ROOT)/components/ble/ble_services/ble_ias_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_lbs \
  $(SDK_ROOT)/components/ble/ble_services/ble_lbs_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_lls \
  $(SDK_ROOT)/components/ble/ble_services/ble_nus \
  $(SDK_ROOT)/components/ble/ble_services/ble_nus_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_rscs \
  $(SDK_ROOT)/components/ble/ble_services/ble_rscs_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_tps \
  $(SDK_ROOT)/components/ble/ble_dtm \
  $(SDK_ROOT)/components/ble/ble_racp \
  $(SDK_ROOT)/components/ble/common \
  $(SDK_ROOT)/components/ble/nrf_ble_qwr \
  $(SDK_ROOT)/components/ble/peer_manager \
  $(SDK_ROOT)/components/libraries/bootloader \
  $(SDK_ROOT)/components/libraries/bootloader/ble_dfu \
  $(SDK_ROOT)/components/libraries/bootloader/dfu \
  $(SDK_ROOT)/components/libraries/bsp \
  $(SDK_ROOT)/components/libraries/button \
  $(SDK_ROOT)/components/libraries/crc16 \
  $(SDK_ROOT)/components/libraries/crc32 \
  $(SDK_ROOT)/components/libraries/crypto \
  $(SDK_ROOT)/components/libraries/csense \
  $(SDK_ROOT)/components/libraries/csense_drv \
  $(SDK_ROOT)/components/libraries/experimental_section_vars \
  $(SDK_ROOT)/components/libraries/fds \
  $(SDK_ROOT)/components/libraries/fifo \
  $(SDK_ROOT)/components/libraries/fstorage \
  $(SDK_ROOT)/components/libraries/gpiote \
  $(SDK_ROOT)/components/libraries/hardfault \
  $(SDK_ROOT)/components/libraries/hci \
  $(SDK_ROOT)/components/libraries/led_softblink \
  $(SDK_ROOT)/components/libraries/log \
  $(SDK_ROOT)/components/libraries/log/src \
  $(SDK_ROOT)/components/libraries/low_power_pwm \
  $(SDK_ROOT)/components/libraries/mem_manager \
  $(SDK_ROOT)/components/libraries/pwm \
  $(SDK_ROOT)/components/libraries/pwr_mgmt \
  $(SDK_ROOT)/components/libraries/queue \
  $(SDK_ROOT)/components/libraries/scheduler \
  $(SDK_ROOT)/components/libraries/sensorsim \
  $(SDK_ROOT)/components/libraries/slip \
  $(SDK_ROOT)/components/libraries/timer \
  $(SDK_ROOT)/components/libraries/twi \
  $(SDK_ROOT)/components/libraries/uart \
  $(SDK_ROOT)/components/libraries/util \
  $(SDK_ROOT)/components/toolchain \
  $(SDK_ROOT)/components/toolchain/cmsis/include \
  $(SDK_ROOT)/components/toolchain/gcc \

# Libraries common to all targets
LIB_FILES += \

include $(COMMON_DIR)/Makefile.common

# Optimization flags release (note - we use -funwind-tables so our app fault handler can reverse engineer the PC of the caller when unsupplied)
OPT = -Os -g3
# Optimization flags debug (important to get line numbers for faults - warning this adds about 40KB to flash size!)
# OPT = -O0 -g3 -DDEBUG_NRF -DDEBUG

# C flags common to all targets
CFLAGS += $(OPT)
CFLAGS += -DBOARD_CUSTOM -DSW102
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DNRF51
CFLAGS += -DNRF_DFU_SETTINGS_VERSION=1
CFLAGS += -DS130
CFLAGS += -DBLE_STACK_SUPPORT_REQD
CFLAGS += -DSWI_DISABLE0
CFLAGS += -DNRF51822
CFLAGS += -DNRF_SD_BLE_API_VERSION=2
CFLAGS += -mcpu=cortex-m0
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -Wall -Werror
CFLAGS += -mfloat-abi=soft
# keep every function in separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin --short-enums

# C++ flags common to all targets
CXXFLAGS += \

# Assembler flags common to all targets
ASMFLAGS += -x assembler-with-cpp
ASMFLAGS += -DBOARD_CUSTOM
ASMFLAGS += -DSOFTDEVICE_PRESENT
ASMFLAGS += -DNRF51
ASMFLAGS += -DNRF_DFU_SETTINGS_VERSION=1
ASMFLAGS += -DS130
ASMFLAGS += -DBLE_STACK_SUPPORT_REQD
ASMFLAGS += -DSWI_DISABLE0
ASMFLAGS += -DNRF51822
ASMFLAGS += -DNRF_SD_BLE_API_VERSION=2

# Linker flags
LDFLAGS += -mthumb -mabi=aapcs -L $(TEMPLATE_PATH) -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m0
# let linker to dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs -lc -lnosys
LDFLAGS += -Xlinker -Map=$(OUTPUT_DIRECTORY)/nrf51822_sw102.map

SOFT_DEVICE = $(SDK_ROOT)/components/softdevice/s130/hex/s130_nrf51_2.0.1_softdevice.hex
# BOOTLOADER_HEX = ../../../SW102_LCD_Bluetooth-bootloader/_build/sw102_bootloader.hex
BOOTLOADER_HEX = prebuilt/sw102_bootloader.hex
KEYFILE := ../../../SW102_LCD_Bluetooth-bootloader/private.key

.PHONY: $(TARGETS) default all clean help flash flash_softdevice

# Default target - first one defined
default: nrf51822_sw102

# Print all targets that can be built
help:
	@echo following targets are available:
	@echo 	nrf51822_sw102

TEMPLATE_PATH := $(SDK_ROOT)/components/toolchain/gcc

include $(TEMPLATE_PATH)/Makefile.common

$(foreach target, $(TARGETS), $(call define_target, $(target)))

# Flash the program
flash_program: $(OUTPUT_DIRECTORY)/nrf51822_sw102.hex
	@echo Flashing: $<
	$(OPENOCD) -c "init; reset init; flash write_image erase $<; verify_image $<; echo FLASHED; reset halt; resume; shutdown"

openocd:
	@echo Starting OPENOCD shell
	$(OPENOCD) -c "init; reset init;"

# Start a command line GDB session
gdb:
	arm-none-eabi-gdb --command=gdb.config

# Start a telnet session talking directly to openocd
openocd-shell:
	telnet localhost 4444

# Flash softdevice
flash_softdevice:
	@echo Flashing: s130_nrf51_2.0.1_softdevice.hex
	$(OPENOCD) -c "init; reset init; nrf51 mass_erase; flash write_image $(SOFT_DEVICE); verify_image $(SDK_ROOT)/components/softdevice/s130/hex/s130_nrf51_2.0.1_softdevice.hex; reset halt; resume; shutdown"

# Erase Flash
erase:
	@echo Erasing all
	$(OPENOCD) -c "init; reset init; nrf51 mass_erase; shutdown"

# Generate DFU package
# Note: don't try the application-version-string option - it just dumbly pulls the string as a nubmer info the int32 in the settings (not what we want)
# Note: we now set debug mode so users can move to prior revisions, not just new ones.
generate_dfu_package: $(OUTPUT_DIRECTORY)/nrf51822_sw102.hex
	mkdir -p $(RELEASE_DIRECTORY)
	$(NRFUTIL) pkg generate --debug-mode --application ./$(OUTPUT_DIRECTORY)/nrf51822_sw102.hex --key-file $(KEYFILE) --application-version $(VERSION_NUM) --hw-version 51 --sd-req 0x87 $(RELEASE_DIRECTORY)/sw102-otaupdate-$(VERSION_STRING).zip

# Generate a prebuilt bin file with bootloader apploand and valid app crc
# per appendix 1 here: https://devzone.nordicsemi.com/nordic/nordic-blog/b/blog/posts/getting-started-with-nordics-secure-dfu-bootloader
# Note: srec_cmp is part of the 'srecord' debian package, for windows supposedly mergehex does the same thing
release_build: generate_dfu_package
	$(NRFUTIL) settings generate --no-backup --family NRF51 --application $(OUTPUT_DIRECTORY)/nrf51822_sw102.hex --application-version $(VERSION_NUM) --bootloader-version 0 --bl-settings-version 1 $(OUTPUT_DIRECTORY)/bootloader_setting.hex
	srec_cat -MULTiple $(BOOTLOADER_HEX) -Intel $(SOFT_DEVICE) -Intel $(OUTPUT_DIRECTORY)/nrf51822_sw102.hex -Intel $(OUTPUT_DIRECTORY)/bootloader_setting.hex -Intel -Output $(RELEASE_DIRECTORY)/sw102-full-$(VERSION_STRING).hex -Intel

# Program a full release build onto the connected device (including bootloader and soft device)
release_program: release_build
	$(OPENOCD) -c "init; reset init; nrf51 mass_erase; flash write_image $(RELEASE_DIRECTORY)/sw102-full-$(VERSION_STRING).hex; verify_image $(RELEASE_DIRECTORY)/sw102-full-$(VERSION_STRING).hex; echo FLASHED; reset halt; resume; shutdown"

# For developers only: program a board with bootloader+softdevice BUT NO APP, to make sure the bootloader will properly boot if flash settings are corrupted
blsd_program: generate_dfu_package
	srec_cat -MULTiple $(BOOTLOADER_HEX) -Intel $(SOFT_DEVICE) -Intel -Output $(RELEASE_DIRECTORY)/sw102-blsdonly-$(VERSION_STRING).hex -Intel
	$(OPENOCD) -c "init; reset init; nrf51 mass_erase; flash write_image $(RELEASE_DIRECTORY)/sw102-blsdonly-$(VERSION_STRING).hex; verify_image $(RELEASE_DIRECTORY)/sw102-blsdonly-$(VERSION_STRING).hex; echo FLASHED; reset halt; resume; shutdown"

# Generate a transitional bootloader package for factory bootloaders (i.e. the devices from @eyebyesickle)
generate_transitional:
	mkdir -p $(RELEASE_DIRECTORY)
	/usr/local/bin/nrfutil dfu genpkg --dfu-ver 0.5 --bootloader $(BOOTLOADER_HEX) --sd-req 100 --softdevice $(SOFT_DEVICE) $(RELEASE_DIRECTORY)/sw102-transitional-$(VERSION_STRING).zip

# Generate an OTA package which can be used to reinstall bootloader and soft device on devices already running our bootloader
# Warning: this is slightly dangerous but useful for testing
generate_bootloader:
	$(NRFUTIL) pkg generate --hw-version 51 --bootloader $(BOOTLOADER_HEX) --bootloader-version 1 --softdevice $(SOFT_DEVICE) --key-file $(KEYFILE) --sd-req 0x87 $(RELEASE_DIRECTORY)/sw102-otabootloader-$(VERSION_STRING).zip

# Start CMSIS_Configuration_Wizard
SDK_CONFIG_FILE := $(PROJ_DIR)/include/sdk_config.h
CMSIS_CONFIG_TOOL := $(SDK_ROOT)/external_tools/cmsisconfig/CMSIS_Configuration_Wizard.jar
sdk_config:
	java -jar $(CMSIS_CONFIG_TOOL) $(SDK_CONFIG_FILE)
