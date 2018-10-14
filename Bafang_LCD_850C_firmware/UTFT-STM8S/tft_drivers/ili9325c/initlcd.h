case ILI9325C:
	UTFT_LCD_Write_COM_DATA(0xE5, 0x78F0); // set SRAM internal timing
	UTFT_LCD_Write_COM_DATA(0x01, 0x0100); // set Driver Output Control  
	UTFT_LCD_Write_COM_DATA(0x02, 0x0700); // set 1 line inversion  
	UTFT_LCD_Write_COM_DATA(0x03, 0x1030); // set GRAM write direction and BGR=1.  
	UTFT_LCD_Write_COM_DATA(0x04, 0x0000); // Resize register  
	UTFT_LCD_Write_COM_DATA(0x08, 0x0207); // set the back porch and front porch  
	UTFT_LCD_Write_COM_DATA(0x09, 0x0000); // set non-display area refresh cycle ISC[3:0]  
	UTFT_LCD_Write_COM_DATA(0x0A, 0x0000); // FMARK function  
	UTFT_LCD_Write_COM_DATA(0x0C, 0x0000); // RGB interface setting  
	UTFT_LCD_Write_COM_DATA(0x0D, 0x0000); // Frame marker Position  
	UTFT_LCD_Write_COM_DATA(0x0F, 0x0000); // RGB interface polarity  
	//*************Power On sequence ****************//  
	UTFT_LCD_Write_COM_DATA(0x10, 0x0000); // SAP, BT[3:0], AP, DSTB, SLP, STB  
	UTFT_LCD_Write_COM_DATA(0x11, 0x0007); // DC1[2:0], DC0[2:0], VC[2:0]  
	UTFT_LCD_Write_COM_DATA(0x12, 0x0000); // VREG1OUT voltage  
	UTFT_LCD_Write_COM_DATA(0x13, 0x0000); // VDV[4:0] for VCOM amplitude  
	UTFT_LCD_Write_COM_DATA(0x07, 0x0001);  
	delay(200); // Dis-charge capacitor power voltage  
	UTFT_LCD_Write_COM_DATA(0x10, 0x1090); // SAP, BT[3:0], AP, DSTB, SLP, STB  
	UTFT_LCD_Write_COM_DATA(0x11, 0x0227); // Set DC1[2:0], DC0[2:0], VC[2:0]  
	delay(50); // Delay 50ms  
	UTFT_LCD_Write_COM_DATA(0x12, 0x001F); // 0012  
	delay(50); // Delay 50ms  
	UTFT_LCD_Write_COM_DATA(0x13, 0x1500); // VDV[4:0] for VCOM amplitude  
	UTFT_LCD_Write_COM_DATA(0x29, 0x0027); // 04  VCM[5:0] for VCOMH  
	UTFT_LCD_Write_COM_DATA(0x2B, 0x000D); // Set Frame Rate  
	delay(50); // Delay 50ms  
	UTFT_LCD_Write_COM_DATA(0x20, 0x0000); // GRAM horizontal Address  
	UTFT_LCD_Write_COM_DATA(0x21, 0x0000); // GRAM Vertical Address  
	// ----------- Adjust the Gamma Curve ----------//  
	UTFT_LCD_Write_COM_DATA(0x30, 0x0000);  
	UTFT_LCD_Write_COM_DATA(0x31, 0x0707);  
	UTFT_LCD_Write_COM_DATA(0x32, 0x0307);  
	UTFT_LCD_Write_COM_DATA(0x35, 0x0200);  
	UTFT_LCD_Write_COM_DATA(0x36, 0x0008);  
	UTFT_LCD_Write_COM_DATA(0x37, 0x0004);  
	UTFT_LCD_Write_COM_DATA(0x38, 0x0000);  
	UTFT_LCD_Write_COM_DATA(0x39, 0x0707);  
	UTFT_LCD_Write_COM_DATA(0x3C, 0x0002);  
	UTFT_LCD_Write_COM_DATA(0x3D, 0x1D04);  
	//------------------ Set GRAM area ---------------//  
	UTFT_LCD_Write_COM_DATA(0x50, 0x0000); // Horizontal GRAM Start Address  
	UTFT_LCD_Write_COM_DATA(0x51, 0x00EF); // Horizontal GRAM End Address  
	UTFT_LCD_Write_COM_DATA(0x52, 0x0000); // Vertical GRAM Start Address  
	UTFT_LCD_Write_COM_DATA(0x53, 0x013F); // Vertical GRAM Start Address  
	UTFT_LCD_Write_COM_DATA(0x60, 0xA700); // Gate Scan Line  
	UTFT_LCD_Write_COM_DATA(0x61, 0x0001); // NDL,VLE, REV   
	UTFT_LCD_Write_COM_DATA(0x6A, 0x0000); // set scrolling line  
	//-------------- Partial Display Control ---------//  
	UTFT_LCD_Write_COM_DATA(0x80, 0x0000);  
	UTFT_LCD_Write_COM_DATA(0x81, 0x0000);  
	UTFT_LCD_Write_COM_DATA(0x82, 0x0000);  
	UTFT_LCD_Write_COM_DATA(0x83, 0x0000);  
	UTFT_LCD_Write_COM_DATA(0x84, 0x0000);  
	UTFT_LCD_Write_COM_DATA(0x85, 0x0000);  
	//-------------- Panel Control -------------------//  
	UTFT_LCD_Write_COM_DATA(0x90, 0x0010);  
	UTFT_LCD_Write_COM_DATA(0x92, 0x0600);  
	UTFT_LCD_Write_COM_DATA(0x07, 0x0133); // 262K color and display ON        
	break;
