/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  /* 4k MBR, 104k Softdevice S130, 127k Application Data, 20k Bootloader, 1k Bootloader Settings */
  FLASH (rx) : ORIGIN = 0x1b000, LENGTH = 256k - 4k - 104k - 20k - 1k
  /* 8k Softdevice S130 */
  RAM (rwx) :  ORIGIN = 0x20002000, LENGTH = 32k - 8k
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