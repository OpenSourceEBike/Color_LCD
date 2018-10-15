case ILI9325C:
  UTFT_LCD_Write_COM_16b (0x10);
//
//
//
//  0x00E5, 0x78F0,     // set SRAM internal timing
//              0x0001, 0x0100,     // set Driver Output Control
//              0x0002, 0x0200,     // set 1 line inversion
//              0x0003, 0x1030,     // set GRAM write direction and BGR=1.
//              0x0004, 0x0000,     // Resize register
//              0x0005, 0x0000,     // .kbv 16bits Data Format Selection
//              0x0008, 0x0207,     // set the back porch and front porch
//              0x0009, 0x0000,     // set non-display area refresh cycle ISC[3:0]
//              0x000A, 0x0000,     // FMARK function
//              0x000C, 0x0000,     // RGB interface setting
//              0x000D, 0x0000,     // Frame marker Position
//              0x000F, 0x0000,     // RGB interface polarity
//              // ----------- Power On sequence ----------- //
//              0x0010, 0x0000,     // SAP, BT[3:0], AP, DSTB, SLP, STB
//              0x0011, 0x0007,     // DC1[2:0], DC0[2:0], VC[2:0]
//              0x0012, 0x0000,     // VREG1OUT voltage
//              0x0013, 0x0000,     // VDV[4:0] for VCOM amplitude
//              0x0007, 0x0001,
//              TFTLCD_DELAY, 200,  // Dis-charge capacitor power voltage
//              0x0010, 0x1690,     // SAP=1, BT=6, APE=1, AP=1, DSTB=0, SLP=0, STB=0
//              0x0011, 0x0227,     // DC1=2, DC0=2, VC=7
//              TFTLCD_DELAY, 50,   // wait_ms 50ms
//              0x0012, 0x000D,     // VCIRE=1, PON=0, VRH=5
//              TFTLCD_DELAY, 50,   // wait_ms 50ms
//              0x0013, 0x1200,     // VDV=28 for VCOM amplitude
//              0x0029, 0x000A,     // VCM=10 for VCOMH
//              0x002B, 0x000D,     // Set Frame Rate
//              TFTLCD_DELAY, 50,   // wait_ms 50ms
//              0x0020, 0x0000,     // GRAM horizontal Address
//              0x0021, 0x0000,     // GRAM Vertical Address
//              // ----------- Adjust the Gamma Curve ----------//
//
//              0x0030, 0x0000,
//              0x0031, 0x0404,
//              0x0032, 0x0003,
//              0x0035, 0x0405,
//              0x0036, 0x0808,
//              0x0037, 0x0407,
//              0x0038, 0x0303,
//              0x0039, 0x0707,
//              0x003C, 0x0504,
//              0x003D, 0x0808,
//
//              //------------------ Set GRAM area ---------------//
//              0x0060, 0x2700,     // Gate Scan Line GS=0 [0xA700]
//              0x0061, 0x0001,     // NDL,VLE, REV .kbv
//              0x006A, 0x0000,     // set scrolling line
//              //-------------- Partial Display Control ---------//
//              0x0080, 0x0000,
//              0x0081, 0x0000,
//              0x0082, 0x0000,
//              0x0083, 0x0000,
//              0x0084, 0x0000,
//              0x0085, 0x0000,
//              //-------------- Panel Control -------------------//
//              0x0090, 0x0010,
//              0x0092, 0x0000,
//              0x0007, 0x0133,     // 262K color and display ON
	break;
