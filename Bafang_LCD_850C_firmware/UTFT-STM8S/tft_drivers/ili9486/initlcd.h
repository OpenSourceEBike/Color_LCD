case ILI9486:
	UTFT_LCD_Write_COM(0x11);		// Sleep OUT
	delay(50);
 
	UTFT_LCD_Write_COM(0xF2);		// ?????
	UTFT_LCD_Write_DATA_VL(0x1C);
	UTFT_LCD_Write_DATA_VL(0xA3);
	UTFT_LCD_Write_DATA_VL(0x32);
	UTFT_LCD_Write_DATA_VL(0x02);
	UTFT_LCD_Write_DATA_VL(0xb2);
	UTFT_LCD_Write_DATA_VL(0x12);
	UTFT_LCD_Write_DATA_VL(0xFF);
	UTFT_LCD_Write_DATA_VL(0x12);
	UTFT_LCD_Write_DATA_VL(0x00);

	UTFT_LCD_Write_COM(0xF1);		// ?????
	UTFT_LCD_Write_DATA_VL(0x36);
	UTFT_LCD_Write_DATA_VL(0xA4);

	UTFT_LCD_Write_COM(0xF8);		// ?????
	UTFT_LCD_Write_DATA_VL(0x21);
	UTFT_LCD_Write_DATA_VL(0x04);

	UTFT_LCD_Write_COM(0xF9);		// ?????
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x08);

	UTFT_LCD_Write_COM(0xC0);		// Power Control 1
	UTFT_LCD_Write_DATA_VL(0x0d);
	UTFT_LCD_Write_DATA_VL(0x0d);

	UTFT_LCD_Write_COM(0xC1);		// Power Control 2
	UTFT_LCD_Write_DATA_VL(0x43);
	UTFT_LCD_Write_DATA_VL(0x00);

	UTFT_LCD_Write_COM(0xC2);		// Power Control 3
	UTFT_LCD_Write_DATA_VL(0x00);

	UTFT_LCD_Write_COM(0xC5);		// VCOM Control
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x48);

	UTFT_LCD_Write_COM(0xB6);		// Display Function Control
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x22);		// 0x42 = Rotate display 180 deg.
	UTFT_LCD_Write_DATA_VL(0x3B);

	UTFT_LCD_Write_COM(0xE0);		// PGAMCTRL (Positive Gamma Control)
	UTFT_LCD_Write_DATA_VL(0x0f);
	UTFT_LCD_Write_DATA_VL(0x24);
	UTFT_LCD_Write_DATA_VL(0x1c);
	UTFT_LCD_Write_DATA_VL(0x0a);
	UTFT_LCD_Write_DATA_VL(0x0f);
	UTFT_LCD_Write_DATA_VL(0x08);
	UTFT_LCD_Write_DATA_VL(0x43);
	UTFT_LCD_Write_DATA_VL(0x88);
	UTFT_LCD_Write_DATA_VL(0x32);
	UTFT_LCD_Write_DATA_VL(0x0f);
	UTFT_LCD_Write_DATA_VL(0x10);
	UTFT_LCD_Write_DATA_VL(0x06);
	UTFT_LCD_Write_DATA_VL(0x0f);
	UTFT_LCD_Write_DATA_VL(0x07);
	UTFT_LCD_Write_DATA_VL(0x00);

	UTFT_LCD_Write_COM(0xE1);		// NGAMCTRL (Negative Gamma Control)
	UTFT_LCD_Write_DATA_VL(0x0F);
	UTFT_LCD_Write_DATA_VL(0x38);
	UTFT_LCD_Write_DATA_VL(0x30);
	UTFT_LCD_Write_DATA_VL(0x09);
	UTFT_LCD_Write_DATA_VL(0x0f);
	UTFT_LCD_Write_DATA_VL(0x0f);
	UTFT_LCD_Write_DATA_VL(0x4e);
	UTFT_LCD_Write_DATA_VL(0x77);
	UTFT_LCD_Write_DATA_VL(0x3c);
	UTFT_LCD_Write_DATA_VL(0x07);
	UTFT_LCD_Write_DATA_VL(0x10);
	UTFT_LCD_Write_DATA_VL(0x05);
	UTFT_LCD_Write_DATA_VL(0x23);
	UTFT_LCD_Write_DATA_VL(0x1b);
	UTFT_LCD_Write_DATA_VL(0x00); 

	UTFT_LCD_Write_COM(0x20);		// Display Inversion OFF
	UTFT_LCD_Write_DATA_VL(0x00);//C8 	 

	UTFT_LCD_Write_COM(0x36);		// Memory Access Control
	UTFT_LCD_Write_DATA_VL(0x0A);

	UTFT_LCD_Write_COM(0x3A);		// Interface Pixel Format
	UTFT_LCD_Write_DATA_VL(0x55); 

	UTFT_LCD_Write_COM(0x2A);		// Column Addess Set
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x01);
	UTFT_LCD_Write_DATA_VL(0xDF);

	UTFT_LCD_Write_COM(0x002B);		// Page Address Set
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x01);
	UTFT_LCD_Write_DATA_VL(0x3f);	 
	delay(50);
	UTFT_LCD_Write_COM(0x0029);		// Display ON
	UTFT_LCD_Write_COM(0x002C);		// Memory Write
	break;
