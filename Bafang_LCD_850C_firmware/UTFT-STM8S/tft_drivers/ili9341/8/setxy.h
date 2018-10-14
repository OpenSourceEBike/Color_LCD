case ILI9341_8:
	UTFT_LCD_Write_COM(0x2A); //column
	UTFT_LCD_Write_DATA(x1>>8,x1);
	UTFT_LCD_Write_DATA(x2>>8,x2);
	UTFT_LCD_Write_COM(0x2B); //page
	UTFT_LCD_Write_DATA(y1>>8,y1);
	UTFT_LCD_Write_DATA(y2>>8,y2);
	UTFT_LCD_Write_COM(0x2C); //write
	break;
