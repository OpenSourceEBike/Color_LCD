case ST7735_8:
	UTFT_LCD_Write_COM(0x2a); 
  	UTFT_LCD_Write_DATA_8(x1>>8);
  	UTFT_LCD_Write_DATA_8(x1);
  	UTFT_LCD_Write_DATA_8(x2>>8);
  	UTFT_LCD_Write_DATA_8(x2);
	UTFT_LCD_Write_COM(0x2b); 
  	UTFT_LCD_Write_DATA_8(y1>>8);
  	UTFT_LCD_Write_DATA_8(y1);
  	UTFT_LCD_Write_DATA_8(y2>>8);
  	UTFT_LCD_Write_DATA_8(y2);
	UTFT_LCD_Write_COM(0x2c); 
	break;
