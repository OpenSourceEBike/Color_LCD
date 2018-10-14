case ST7735:
	UTFT_LCD_Write_COM(0x11);//Sleep exit 
	delay(12);
 
	//ST7735R Frame Rate
	UTFT_LCD_Write_COM(0xB1); 
	UTFT_LCD_Write_DATA_VL(0x01);
	UTFT_LCD_Write_DATA_VL(0x2C);
	UTFT_LCD_Write_DATA_VL(0x2D); 
	UTFT_LCD_Write_COM(0xB2); 
	UTFT_LCD_Write_DATA_VL(0x01);
	UTFT_LCD_Write_DATA_VL(0x2C);
	UTFT_LCD_Write_DATA_VL(0x2D); 
	UTFT_LCD_Write_COM(0xB3); 
	UTFT_LCD_Write_DATA_VL(0x01);
	UTFT_LCD_Write_DATA_VL(0x2C);
	UTFT_LCD_Write_DATA_VL(0x2D); 
	UTFT_LCD_Write_DATA_VL(0x01);
	UTFT_LCD_Write_DATA_VL(0x2C);
	UTFT_LCD_Write_DATA_VL(0x2D); 

	UTFT_LCD_Write_COM(0xB4); //Column inversion 
	UTFT_LCD_Write_DATA_VL(0x07); 
 
	//ST7735R Power Sequence
	UTFT_LCD_Write_COM(0xC0); 
	UTFT_LCD_Write_DATA_VL(0xA2);
	UTFT_LCD_Write_DATA_VL(0x02);
	UTFT_LCD_Write_DATA_VL(0x84); 
	UTFT_LCD_Write_COM(0xC1);
	UTFT_LCD_Write_DATA_VL(0xC5); 
	UTFT_LCD_Write_COM(0xC2); 
	UTFT_LCD_Write_DATA_VL(0x0A);
	UTFT_LCD_Write_DATA_VL(0x00); 
	UTFT_LCD_Write_COM(0xC3); 
	UTFT_LCD_Write_DATA_VL(0x8A);
	UTFT_LCD_Write_DATA_VL(0x2A); 
	UTFT_LCD_Write_COM(0xC4); 
	UTFT_LCD_Write_DATA_VL(0x8A);
	UTFT_LCD_Write_DATA_VL(0xEE); 
 
	UTFT_LCD_Write_COM(0xC5); //VCOM 
	UTFT_LCD_Write_DATA_VL(0x0E); 
 
	UTFT_LCD_Write_COM(0x36); //MX, MY, RGB mode 
	UTFT_LCD_Write_DATA_VL(0xC8); 

	//ST7735R Gamma Sequence
	UTFT_LCD_Write_COM(0xe0); 
	UTFT_LCD_Write_DATA_VL(0x0f);
	UTFT_LCD_Write_DATA_VL(0x1a); 
	UTFT_LCD_Write_DATA_VL(0x0f);
	UTFT_LCD_Write_DATA_VL(0x18); 
	UTFT_LCD_Write_DATA_VL(0x2f);
	UTFT_LCD_Write_DATA_VL(0x28); 
	UTFT_LCD_Write_DATA_VL(0x20);
	UTFT_LCD_Write_DATA_VL(0x22); 
	UTFT_LCD_Write_DATA_VL(0x1f);
	UTFT_LCD_Write_DATA_VL(0x1b); 
	UTFT_LCD_Write_DATA_VL(0x23);
	UTFT_LCD_Write_DATA_VL(0x37);
	UTFT_LCD_Write_DATA_VL(0x00); 

	UTFT_LCD_Write_DATA_VL(0x07); 
	UTFT_LCD_Write_DATA_VL(0x02);
	UTFT_LCD_Write_DATA_VL(0x10); 
	UTFT_LCD_Write_COM(0xe1); 
	UTFT_LCD_Write_DATA_VL(0x0f);
	UTFT_LCD_Write_DATA_VL(0x1b); 
	UTFT_LCD_Write_DATA_VL(0x0f);
	UTFT_LCD_Write_DATA_VL(0x17); 
	UTFT_LCD_Write_DATA_VL(0x33);
	UTFT_LCD_Write_DATA_VL(0x2c); 
	UTFT_LCD_Write_DATA_VL(0x29);
	UTFT_LCD_Write_DATA_VL(0x2e); 
	UTFT_LCD_Write_DATA_VL(0x30);
	UTFT_LCD_Write_DATA_VL(0x30); 
	UTFT_LCD_Write_DATA_VL(0x39);
	UTFT_LCD_Write_DATA_VL(0x3f); 
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x07); 
	UTFT_LCD_Write_DATA_VL(0x03);
	UTFT_LCD_Write_DATA_VL(0x10);  

	UTFT_LCD_Write_COM(0x2a);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x7f);
	UTFT_LCD_Write_COM(0x2b);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x9f);

	UTFT_LCD_Write_COM(0xF0); //Enable test command  
	UTFT_LCD_Write_DATA_VL(0x01); 
	UTFT_LCD_Write_COM(0xF6); //Disable ram power save mode 
	UTFT_LCD_Write_DATA_VL(0x00); 
 
	UTFT_LCD_Write_COM(0x3A); //65k mode 
	UTFT_LCD_Write_DATA_VL(0x05); 
	UTFT_LCD_Write_COM(0x29);//Display on
	break;
