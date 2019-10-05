# Notes about the stock factory firmware


## UICR & FICR registers

headerfile says the layout is (they are using version 9 of the sdk)
```
typedef struct {                                    /*!< UICR Structure                                                        */
  __IO uint32_t  CLENR0;                            /*!< Length of code region 0.                                              */
  __IO uint32_t  RBPCONF;                           /*!< Readback protection configuration.                                    */
  __IO uint32_t  XTALFREQ;                          /*!< Reset value for CLOCK XTALFREQ register.                              */
  __I  uint32_t  RESERVED0;
  __I  uint32_t  FWID;                              /*!< Firmware ID.                                                          */

  union {
    __IO uint32_t  NRFFW[15];                       /*!< Reserved for Nordic firmware design.                                  */
    __IO uint32_t  BOOTLOADERADDR;                  /*!< Bootloader start address.                                             */
  };
  __IO uint32_t  NRFHW[12];                         /*!< Reserved for Nordic hardware design.                                  */
  __IO uint32_t  CUSTOMER[32];                      /*!< Reserved for customer.                                                */
} NRF_UICR_Type;
```

0x10001000 means the protected coderegion 0 goes from 0 to 0x18000 (soft device).  Note: our newer S130 soft device goes to 0x1b000.
0x10001004 means readout protected (use 0xffffffff to allow reading)
1014 means bootloader starts at 3b000.

from board under testing
(gdb) x/64wx 0x10001000
0x10001000:	0x00018000	0xffff00ff	0xffffffff	0xffffffff
0x10001010:	0xffffffff	0x0003b000	0xffffffff	0xffffffff
0x10001020:	0xffffffff	0xffffffff	0xffffffff	0xffffffff
0x10001030:	0xffffffff	0xffffffff	0xffffffff	0xffffffff
0x10001040:	0xffffffff	0xffffffff	0xffffffff	0xffffffff
0x10001050:	0xffffffff	0xffffffff	0xffffffff	0xffffffff
0x10001060:	0xffffffff	0xffffffff	0xffffffff	0xffffffff
0x10001070:	0xffffffff	0xffffffff	0xffffffff	0xffffffff
0x10001080:	0xffffffff	0xffffffff	0xffffffff	0xffffffff
0x10001090:	0xffffffff	0xffffffff	0xffffffff	0xffffffff
0x100010a0:	0xffffffff	0xffffffff	0xffffffff	0xffffffff
0x100010b0:	0xffffffff	0xffffffff	0xffffffff	0xffffffff
0x100010c0:	0xffffffff	0xffffffff	0xffffffff	0xffffffff
0x100010d0:	0xffffffff	0xffffffff	0xffffffff	0xffffffff
0x100010e0:	0xffffffff	0xffffffff	0xffffffff	0xffffffff
0x100010f0:	0xffffffff	0xffffffff	0xffffffff	0xffffffff
(gdb)
(gdb) x/64wx 0x10000000
0x10000000:	0x55aa55aa	0x55aa55aa	0xffffffff	0xffffffff
0x10000010:	0x00000400	0x00000100	0xffffffff	0xffffffff
0x10000020:	0xffffffff	0xffffffff	0xffffffff	0xffffffff
0x10000030:	0xffffff00	0x00000004	0x00002000	0x00002000
0x10000040:	0x00002000	0x00002000	0x8863760f	0x9a8bf17e
0x10000050:	0xfffffff8	0xffffffff	0x00000000	0xffff0083
0x10000060:	0x2dd37a2d	0x1cd00a96	0xffffffff	0xffffffff
0x10000070:	0x34524e43	0x0c0d3738	0xffffff16	0xffffffff
0x10000080:	0x5dd683bb	0x62a6e47a	0x38d5a56f	0x6faef9ae
0x10000090:	0xbfd68656	0x7e439310	0x6d9dda9e	0x2901ef4d
0x100000a0:	0xffffffff	0xdc492d67	0xc2074764	0xfffffff6
0x100000b0:	0x78005000	0x5400004e	0x600c8005	0x00726424
0x100000c0:	0x8203423e	0xffffffff	0xffffffff	0xffffffff
0x100000d0:	0xffffffff	0xffffffff	0xffffffff	0xffffffff
0x100000e0:	0xffffffff	0xffffffff	0xffffffff	0x7d005200
0x100000f0:	0x5c000050	0x680e8806	0x00726424	0x8253423e
(gdb)
