case ILI9341_8:
	UTFT_LCD_Write_COM(0xcf); 
	UTFT_LCD_Write_DATA_8(0x00);
	UTFT_LCD_Write_DATA_8(0xc1);
	UTFT_LCD_Write_DATA_8(0x30);

	UTFT_LCD_Write_COM(0xed); 
	UTFT_LCD_Write_DATA_8(0x64);
	UTFT_LCD_Write_DATA_8(0x03);
	UTFT_LCD_Write_DATA_8(0x12);
	UTFT_LCD_Write_DATA_8(0x81);

	UTFT_LCD_Write_COM(0xcb); 
	UTFT_LCD_Write_DATA_8(0x39);
	UTFT_LCD_Write_DATA_8(0x2c);
	UTFT_LCD_Write_DATA_8(0x00);
	UTFT_LCD_Write_DATA_8(0x34);
	UTFT_LCD_Write_DATA_8(0x02);

	UTFT_LCD_Write_COM(0xea); 
	UTFT_LCD_Write_DATA_8(0x00);
	UTFT_LCD_Write_DATA_8(0x00);

	UTFT_LCD_Write_COM(0xe8); 
	UTFT_LCD_Write_DATA_8(0x85);
	UTFT_LCD_Write_DATA_8(0x10);
	UTFT_LCD_Write_DATA_8(0x79);

	UTFT_LCD_Write_COM(0xC0); //Power control
	UTFT_LCD_Write_DATA_8(0x23); //VRH[5:0]

	UTFT_LCD_Write_COM(0xC1); //Power control
	UTFT_LCD_Write_DATA_8(0x11); //SAP[2:0];BT[3:0]

	UTFT_LCD_Write_COM(0xC2);
	UTFT_LCD_Write_DATA_8(0x11);

	UTFT_LCD_Write_COM(0xC5); //VCM control
	UTFT_LCD_Write_DATA_8(0x3d);
	UTFT_LCD_Write_DATA_8(0x30);

	UTFT_LCD_Write_COM(0xc7); 
	UTFT_LCD_Write_DATA_8(0xaa);

	UTFT_LCD_Write_COM(0x3A); 
	UTFT_LCD_Write_DATA_8(0x55);

	UTFT_LCD_Write_COM(0x36); // Memory Access Control
	UTFT_LCD_Write_DATA_8(0x08);

	UTFT_LCD_Write_COM(0xB1); // Frame Rate Control
	UTFT_LCD_Write_DATA_8(0x00);
	UTFT_LCD_Write_DATA_8(0x11);

	UTFT_LCD_Write_COM(0xB6); // Display Function Control
	UTFT_LCD_Write_DATA_8(0x0a);
	UTFT_LCD_Write_DATA_8(0xa2);

	UTFT_LCD_Write_COM(0xF2); // 3Gamma Function Disable
	UTFT_LCD_Write_DATA_8(0x00);

	UTFT_LCD_Write_COM(0xF7);
	UTFT_LCD_Write_DATA_8(0x20);

	UTFT_LCD_Write_COM(0xF1);
	UTFT_LCD_Write_DATA_8(0x01);
	UTFT_LCD_Write_DATA_8(0x30);

	UTFT_LCD_Write_COM(0x26); //Gamma curve selected
	UTFT_LCD_Write_DATA_8(0x01);

	UTFT_LCD_Write_COM(0xE0); //Set Gamma
	UTFT_LCD_Write_DATA_8(0x0f);
	UTFT_LCD_Write_DATA_8(0x3f);
	UTFT_LCD_Write_DATA_8(0x2f);
	UTFT_LCD_Write_DATA_8(0x0c);
	UTFT_LCD_Write_DATA_8(0x10);
	UTFT_LCD_Write_DATA_8(0x0a);
	UTFT_LCD_Write_DATA_8(0x53);
	UTFT_LCD_Write_DATA_8(0xd5);
	UTFT_LCD_Write_DATA_8(0x40);
	UTFT_LCD_Write_DATA_8(0x0a);
	UTFT_LCD_Write_DATA_8(0x13);
	UTFT_LCD_Write_DATA_8(0x03);
	UTFT_LCD_Write_DATA_8(0x08);
	UTFT_LCD_Write_DATA_8(0x03);
	UTFT_LCD_Write_DATA_8(0x00);

	UTFT_LCD_Write_COM(0xE1); //Set Gamma
	UTFT_LCD_Write_DATA_8(0x00);
	UTFT_LCD_Write_DATA_8(0x00);
	UTFT_LCD_Write_DATA_8(0x10);
	UTFT_LCD_Write_DATA_8(0x03);
	UTFT_LCD_Write_DATA_8(0x0f);
	UTFT_LCD_Write_DATA_8(0x05);
	UTFT_LCD_Write_DATA_8(0x2c);
	UTFT_LCD_Write_DATA_8(0xa2);
	UTFT_LCD_Write_DATA_8(0x3f);
	UTFT_LCD_Write_DATA_8(0x05);
	UTFT_LCD_Write_DATA_8(0x0e);
	UTFT_LCD_Write_DATA_8(0x0c);
	UTFT_LCD_Write_DATA_8(0x37);
	UTFT_LCD_Write_DATA_8(0x3c);
	UTFT_LCD_Write_DATA_8(0x0F);
	UTFT_LCD_Write_COM(0x11); //Exit Sleep
	delay(120);
	UTFT_LCD_Write_COM(0x29); //display on
	UTFT_LCD_Write_COM(0x2c); //display on
	delay(50);
	break;


