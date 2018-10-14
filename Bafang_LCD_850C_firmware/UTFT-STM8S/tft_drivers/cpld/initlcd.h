case CPLD:
	if (UTFT_orient==LANDSCAPE)
		UTFT_orient = PORTRAIT;
	else
		UTFT_orient = LANDSCAPE;

	UTFT_LCD_Write_COM(0x0F);   
	UTFT_LCD_Write_COM_DATA(0x01,0x0010);
	UTFT_LCD_Write_COM(0x0F);   
	break;
