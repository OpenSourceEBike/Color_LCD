case ILI9481:
  //************* Reset LCD Driver ****************//
  GPIO_SetBits(LCD_RESET__PORT, LCD_RESET__PIN);
  delay(10);
  GPIO_ResetBits(LCD_RESET__PORT, LCD_RESET__PIN);
  delay(25);
  GPIO_SetBits(LCD_RESET__PORT, LCD_RESET__PIN);
  delay(250);

  UTFT_LCD_Write_COM(0xD0); // Power Setting
  UTFT_LCD_Write_DATA_VL(0x07);
  UTFT_LCD_Write_DATA_VL(0x41);
  UTFT_LCD_Write_DATA_VL(0x1D);

  UTFT_LCD_Write_COM(0xD2); // Power_Setting for Normal Mode
  UTFT_LCD_Write_DATA_VL(0x01);
  UTFT_LCD_Write_DATA_VL(0x11);

  UTFT_LCD_Write_COM(0xC0); // Panel Driving Setting
  UTFT_LCD_Write_DATA_VL(0x10);
  UTFT_LCD_Write_DATA_VL(0x3B);
  UTFT_LCD_Write_DATA_VL(0x00);
  UTFT_LCD_Write_DATA_VL(0x02);
  UTFT_LCD_Write_DATA_VL(0x11);

  UTFT_LCD_Write_COM(0xC5); // Frame rate and Inversion Control
  UTFT_LCD_Write_DATA_VL(0x00);

  UTFT_LCD_Write_COM(0xE4); // ????
  UTFT_LCD_Write_DATA_VL(0xA0);

  UTFT_LCD_Write_COM(0xF0); // ??
  UTFT_LCD_Write_DATA_VL(0x01);

  UTFT_LCD_Write_COM(0xF3); // ??
  UTFT_LCD_Write_DATA_VL(0x40);
  UTFT_LCD_Write_DATA_VL(0x1A);

  UTFT_LCD_Write_COM(0xC8); // Gamma Setting
  UTFT_LCD_Write_DATA_VL(0x00);
  UTFT_LCD_Write_DATA_VL(0x14);
  UTFT_LCD_Write_DATA_VL(0x33);
  UTFT_LCD_Write_DATA_VL(0x10);
  UTFT_LCD_Write_DATA_VL(0x00);
  UTFT_LCD_Write_DATA_VL(0x16);
  UTFT_LCD_Write_DATA_VL(0x44);
  UTFT_LCD_Write_DATA_VL(0x36);
  UTFT_LCD_Write_DATA_VL(0x77);
  UTFT_LCD_Write_DATA_VL(0x00);
  UTFT_LCD_Write_DATA_VL(0x0F);
  UTFT_LCD_Write_DATA_VL(0x00);

  UTFT_LCD_Write_COM(0x3A); // set_pixel_format
  UTFT_LCD_Write_DATA_VL(0x55); // 16bit/pixel (65,536 colors)

  UTFT_LCD_Write_COM(0x11); // exit_sleep_mode

  delay(120); // 120ms delay after leaving sleep

  UTFT_LCD_Write_COM(0x29); // set_display_on

  UTFT_LCD_Write_COM(0x36); // set_address_mode
  // Vertical Flip: Normal display
  // Horizontal Flip: Flipped display
  // RGB/BGR Order: Pixels sent in BGR order
  // Column Address Order: Right to Left
  // Page Address Order: Top to Bottom
  UTFT_LCD_Write_DATA_VL(0x0A);

  UTFT_LCD_Write_COM(0x2A); // set_column_address
  // 0x1df = 319
  UTFT_LCD_Write_DATA_VL(0x00);
  UTFT_LCD_Write_DATA_VL(0x00);
  UTFT_LCD_Write_DATA_VL(0x01);
  UTFT_LCD_Write_DATA_VL(0x3F);

  UTFT_LCD_Write_COM(0x2B); // set_page_address
  // 0x1df = 479
  UTFT_LCD_Write_DATA_VL(0x00);
  UTFT_LCD_Write_DATA_VL(0x00);
  UTFT_LCD_Write_DATA_VL(0x01);
  UTFT_LCD_Write_DATA_VL(0xDF);

  UTFT_LCD_Write_COM(0x2C);

  break;






//case ILI9481:
//	UTFT_LCD_Write_COM(0x11);
//	delay(20);
//	UTFT_LCD_Write_COM(0xD0);
//	UTFT_LCD_Write_DATA_VL(0x07);
//	UTFT_LCD_Write_DATA_VL(0x42);
//	UTFT_LCD_Write_DATA_VL(0x18);
//
//	UTFT_LCD_Write_COM(0xD1);
//	UTFT_LCD_Write_DATA_VL(0x00);
//	UTFT_LCD_Write_DATA_VL(0x07);
//	UTFT_LCD_Write_DATA_VL(0x10);
//
//	UTFT_LCD_Write_COM(0xD2);
//	UTFT_LCD_Write_DATA_VL(0x01);
//	UTFT_LCD_Write_DATA_VL(0x02);
//
//	UTFT_LCD_Write_COM(0xC0);
//	UTFT_LCD_Write_DATA_VL(0x10);
//	UTFT_LCD_Write_DATA_VL(0x3B);
//	UTFT_LCD_Write_DATA_VL(0x00);
//	UTFT_LCD_Write_DATA_VL(0x02);
//	UTFT_LCD_Write_DATA_VL(0x11);
//
//	UTFT_LCD_Write_COM(0xC5);
//	UTFT_LCD_Write_DATA_VL(0x03);
//
//	UTFT_LCD_Write_COM(0xC8);
//	UTFT_LCD_Write_DATA_VL(0x00);
//	UTFT_LCD_Write_DATA_VL(0x32);
//	UTFT_LCD_Write_DATA_VL(0x36);
//	UTFT_LCD_Write_DATA_VL(0x45);
//	UTFT_LCD_Write_DATA_VL(0x06);
//	UTFT_LCD_Write_DATA_VL(0x16);
//	UTFT_LCD_Write_DATA_VL(0x37);
//	UTFT_LCD_Write_DATA_VL(0x75);
//	UTFT_LCD_Write_DATA_VL(0x77);
//	UTFT_LCD_Write_DATA_VL(0x54);
//	UTFT_LCD_Write_DATA_VL(0x0C);
//	UTFT_LCD_Write_DATA_VL(0x00);
//
//	UTFT_LCD_Write_COM(0x36);
//	UTFT_LCD_Write_DATA_VL(0x0A);
//
//
//	UTFT_LCD_Write_COM(0x3A);
//	UTFT_LCD_Write_DATA_VL(0x55);
//
//	UTFT_LCD_Write_COM(0x2A);
//	UTFT_LCD_Write_DATA_VL(0x00);
//	UTFT_LCD_Write_DATA_VL(0x00);
//	UTFT_LCD_Write_DATA_VL(0x01);
//	UTFT_LCD_Write_DATA_VL(0x3F);
//
//	UTFT_LCD_Write_COM(0x2B);
//	UTFT_LCD_Write_DATA_VL(0x00);
//	UTFT_LCD_Write_DATA_VL(0x00);
//	UTFT_LCD_Write_DATA_VL(0x01);
//	UTFT_LCD_Write_DATA_VL(0xE0);
//	delay(120);
//	UTFT_LCD_Write_COM(0x29);
//	break;
