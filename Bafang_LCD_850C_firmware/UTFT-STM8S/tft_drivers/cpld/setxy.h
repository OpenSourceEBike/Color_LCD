case CPLD:
	UTFT_LCD_Write_COM_DATA(0x02, y1);
	UTFT_LCD_Write_COM_DATA(0x03, x1);
   	UTFT_LCD_Write_COM_DATA(0x06, y2);
	UTFT_LCD_Write_COM_DATA(0x07, x2);
	UTFT_LCD_Write_COM(0x0F);					 						 
	break;
