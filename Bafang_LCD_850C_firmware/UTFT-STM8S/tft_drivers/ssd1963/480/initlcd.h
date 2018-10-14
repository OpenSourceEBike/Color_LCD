case SSD1963_480:
	UTFT_LCD_Write_COM(0xE2);		//PLL multiplier, set PLL clock to 120M
	UTFT_LCD_Write_DATA_VL(0x23);	    //N=0x36 for 6.5M, 0x23 for 10M crystal
	UTFT_LCD_Write_DATA_VL(0x02);
	UTFT_LCD_Write_DATA_VL(0x54);
	UTFT_LCD_Write_COM(0xE0);		// PLL enable
	UTFT_LCD_Write_DATA_VL(0x01);
	delay(10);
	UTFT_LCD_Write_COM(0xE0);
	UTFT_LCD_Write_DATA_VL(0x03);
	delay(10);
	UTFT_LCD_Write_COM(0x01);		// software reset
	delay(100);
	UTFT_LCD_Write_COM(0xE6);		//PLL setting for PCLK, depends on resolution
	UTFT_LCD_Write_DATA_VL(0x01);
	UTFT_LCD_Write_DATA_VL(0x1F);
	UTFT_LCD_Write_DATA_VL(0xFF);

	UTFT_LCD_Write_COM(0xB0);		//LCD SPECIFICATION
	UTFT_LCD_Write_DATA_VL(0x20);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x01);		//Set HDP	479
	UTFT_LCD_Write_DATA_VL(0xDF);
	UTFT_LCD_Write_DATA_VL(0x01);		//Set VDP	271
	UTFT_LCD_Write_DATA_VL(0x0F);
	UTFT_LCD_Write_DATA_VL(0x00);

	UTFT_LCD_Write_COM(0xB4);		//HSYNC
	UTFT_LCD_Write_DATA_VL(0x02);		//Set HT	531
	UTFT_LCD_Write_DATA_VL(0x13);
	UTFT_LCD_Write_DATA_VL(0x00);		//Set HPS	8
	UTFT_LCD_Write_DATA_VL(0x08);
	UTFT_LCD_Write_DATA_VL(0x2B);		//Set HPW	43
	UTFT_LCD_Write_DATA_VL(0x00);		//Set LPS	2
	UTFT_LCD_Write_DATA_VL(0x02);
	UTFT_LCD_Write_DATA_VL(0x00);

	UTFT_LCD_Write_COM(0xB6);		//VSYNC
	UTFT_LCD_Write_DATA_VL(0x01);		//Set VT	288
	UTFT_LCD_Write_DATA_VL(0x20);
	UTFT_LCD_Write_DATA_VL(0x00);		//Set VPS	4
	UTFT_LCD_Write_DATA_VL(0x04);
	UTFT_LCD_Write_DATA_VL(0x0c);		//Set VPW	12
	UTFT_LCD_Write_DATA_VL(0x00);		//Set FPS	2
	UTFT_LCD_Write_DATA_VL(0x02);

	UTFT_LCD_Write_COM(0xBA);
	UTFT_LCD_Write_DATA_VL(0x0F);		//GPIO[3:0] out 1

	UTFT_LCD_Write_COM(0xB8);
	UTFT_LCD_Write_DATA_VL(0x07);	    //GPIO3=input, GPIO[2:0]=output
	UTFT_LCD_Write_DATA_VL(0x01);		//GPIO0 normal

	UTFT_LCD_Write_COM(0x36);		//rotation
	UTFT_LCD_Write_DATA_VL(0x22);

	UTFT_LCD_Write_COM(0xF0);		//pixel data interface
	UTFT_LCD_Write_DATA_VL(0x03);


	delay(1);

	UTFT_setXY(0, 0, 479, 271);
	UTFT_LCD_Write_COM(0x29);		//display on

	UTFT_LCD_Write_COM(0xBE);		//set PWM for B/L
	UTFT_LCD_Write_DATA_VL(0x06);
	UTFT_LCD_Write_DATA_VL(0xf0);
	UTFT_LCD_Write_DATA_VL(0x01);
	UTFT_LCD_Write_DATA_VL(0xf0);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x00);

	UTFT_LCD_Write_COM(0xd0); 
	UTFT_LCD_Write_DATA_VL(0x0d);	

	UTFT_LCD_Write_COM(0x2C); 
	break;
