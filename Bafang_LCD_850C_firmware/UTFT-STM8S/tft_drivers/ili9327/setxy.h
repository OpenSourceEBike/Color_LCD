case ILI9327:
	UTFT_LCD_Write_COM(0x2a);
  	UTFT_LCD_Write_DATA(0x00,x1>>8);
  	UTFT_LCD_Write_DATA(0x00,x1);
  	UTFT_LCD_Write_DATA(0x00,x2>>8);
  	UTFT_LCD_Write_DATA(0x00,x2);
  	UTFT_LCD_Write_COM(0x2b);
  	UTFT_LCD_Write_DATA(0x00,y1>>8);
  	UTFT_LCD_Write_DATA(0x00,y1);
  	UTFT_LCD_Write_DATA(0x00,y2>>8);
  	UTFT_LCD_Write_DATA(0x00,y2);
  	UTFT_LCD_Write_COM(0x2c); 							 
	break;
