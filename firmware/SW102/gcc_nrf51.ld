/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  /* 4k MBR, 104k Softdevice S130, 127k Application Data, 20k Bootloader, 1k Bootloader Settings */
  FLASH (rx)      : ORIGIN = DEFINED(USE_WITH_BOOTLOADER) ? (0x1b000) : 0x0, LENGTH = DEFINED(USE_WITH_BOOTLOADER) ? (256k - 4k - 104k - 20k - 1k) : 256k
  /* 11k Softdevice S130 */
  RAM (xrw)       : ORIGIN = DEFINED(USE_WITH_BOOTLOADER) ? (0x20002C00) : 0x20000000, LENGTH = DEFINED(USE_WITH_BOOTLOADER) ? (32k - 11k) : 32k
}

SECTIONS
{
  .fs_data :
  {
    PROVIDE(__start_fs_data = .);
    KEEP(*(.fs_data))
    PROVIDE(__stop_fs_data = .);
  } > RAM
  .pwr_mgmt_data :
  {
    PROVIDE(__start_pwr_mgmt_data = .);
    KEEP(*(.pwr_mgmt_data))
    PROVIDE(__stop_pwr_mgmt_data = .);
  } > RAM
} INSERT AFTER .data;

INCLUDE "nrf5x_common.ld"
