case SSD1963_800:
	swap(word, x1, y1);
	swap(word, x2, y2);
	UTFT_LCD_Write_COM(0x2a); 
  	UTFT_LCD_Write_DATA_VL(x1>>8);
  	UTFT_LCD_Write_DATA_VL(x1);
  	UTFT_LCD_Write_DATA_VL(x2>>8);
  	UTFT_LCD_Write_DATA_VL(x2);
	UTFT_LCD_Write_COM(0x2b); 
  	UTFT_LCD_Write_DATA_VL(y1>>8);
  	UTFT_LCD_Write_DATA_VL(y1);
  	UTFT_LCD_Write_DATA_VL(y2>>8);
  	UTFT_LCD_Write_DATA_VL(y2);
	UTFT_LCD_Write_COM(0x2c); 
	break;
