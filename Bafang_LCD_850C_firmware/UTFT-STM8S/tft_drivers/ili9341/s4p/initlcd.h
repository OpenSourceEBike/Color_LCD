case ILI9341_S4P:
	UTFT_LCD_Write_COM(0x11);//sleep out 
	delay(20);
  //UTFT_LCD_Write_COM(0x01); //reset
  delay(15);
  UTFT_LCD_Write_COM(0x28); //display off
  delay(5);
  UTFT_LCD_Write_COM(0xCF); //power control b
  UTFT_LCD_Write_DATA_VL(0x00);
  UTFT_LCD_Write_DATA_VL(0x83); //83 81 AA
  UTFT_LCD_Write_DATA_VL(0x30);
  UTFT_LCD_Write_COM(0xED); //power on seq control
  UTFT_LCD_Write_DATA_VL(0x64); //64 67
  UTFT_LCD_Write_DATA_VL(0x03);
  UTFT_LCD_Write_DATA_VL(0x12);
  UTFT_LCD_Write_DATA_VL(0x81);
  UTFT_LCD_Write_COM(0xE8); //timing control a
  UTFT_LCD_Write_DATA_VL(0x85);
  UTFT_LCD_Write_DATA_VL(0x01);
  UTFT_LCD_Write_DATA_VL(0x79); //79 78
  UTFT_LCD_Write_COM(0xCB); //power control a
  UTFT_LCD_Write_DATA_VL(0x39);
  UTFT_LCD_Write_DATA_VL(0X2C);
  UTFT_LCD_Write_DATA_VL(0x00);
  UTFT_LCD_Write_DATA_VL(0x34);
  UTFT_LCD_Write_DATA_VL(0x02);
  UTFT_LCD_Write_COM(0xF7); //pump ratio control
  UTFT_LCD_Write_DATA_VL(0x20);
  UTFT_LCD_Write_COM(0xEA); //timing control b
  UTFT_LCD_Write_DATA_VL(0x00);
  UTFT_LCD_Write_DATA_VL(0x00);
  UTFT_LCD_Write_COM(0xC0); //power control 2
  UTFT_LCD_Write_DATA_VL(0x26); //26 25
  UTFT_LCD_Write_COM(0xC1); //power control 2
  UTFT_LCD_Write_DATA_VL(0x11);
  UTFT_LCD_Write_COM(0xC5); //vcom control 1
  UTFT_LCD_Write_DATA_VL(0x35);
  UTFT_LCD_Write_DATA_VL(0x3E);
  UTFT_LCD_Write_COM(0xC7); //vcom control 2
  UTFT_LCD_Write_DATA_VL(0xBE); //BE 94
  UTFT_LCD_Write_COM(0xB1); //frame control
  UTFT_LCD_Write_DATA_VL(0x00);
  UTFT_LCD_Write_DATA_VL(0x1B); //1B 70
  UTFT_LCD_Write_COM(0xB6); //display control
  UTFT_LCD_Write_DATA_VL(0x0A);
  UTFT_LCD_Write_DATA_VL(0x82);
  UTFT_LCD_Write_DATA_VL(0x27);
  UTFT_LCD_Write_DATA_VL(0x00);
  UTFT_LCD_Write_COM(0xB7); //emtry mode
  UTFT_LCD_Write_DATA_VL(0x07);
  UTFT_LCD_Write_COM(0x3A); //pixel format
  UTFT_LCD_Write_DATA_VL(0x55); //16bit
  UTFT_LCD_Write_COM(0x36); //mem access
  UTFT_LCD_Write_DATA_VL((1<<3)|(1<<6));
  //UTFT_LCD_Write_DATA_VL((1<<3)|(1<<7)); //rotate 180
  UTFT_LCD_Write_COM(0x29); //display on
  delay(5);
	break;
