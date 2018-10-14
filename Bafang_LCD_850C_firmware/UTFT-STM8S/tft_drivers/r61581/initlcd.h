case R61581:
		UTFT_LCD_Write_COM(0xB0);		
		UTFT_LCD_Write_DATA_VL(0x1E);	    

		UTFT_LCD_Write_COM(0xB0);
		UTFT_LCD_Write_DATA_VL(0x00);

		UTFT_LCD_Write_COM(0xB3);
		UTFT_LCD_Write_DATA_VL(0x02);
		UTFT_LCD_Write_DATA_VL(0x00);
		UTFT_LCD_Write_DATA_VL(0x00);
		UTFT_LCD_Write_DATA_VL(0x10);

		UTFT_LCD_Write_COM(0xB4);
		UTFT_LCD_Write_DATA_VL(0x00);//0X10

// 		UTFT_LCD_Write_COM(0xB9); //PWM Settings for Brightness Control
// 		UTFT_LCD_Write_DATA_VL(0x01);// Disabled by default. 
// 		UTFT_LCD_Write_DATA_VL(0xFF); //0xFF = Max brightness
// 		UTFT_LCD_Write_DATA_VL(0xFF);
// 		UTFT_LCD_Write_DATA_VL(0x18);

		UTFT_LCD_Write_COM(0xC0);
		UTFT_LCD_Write_DATA_VL(0x03);
		UTFT_LCD_Write_DATA_VL(0x3B);//
		UTFT_LCD_Write_DATA_VL(0x00);
		UTFT_LCD_Write_DATA_VL(0x00);
		UTFT_LCD_Write_DATA_VL(0x00);
		UTFT_LCD_Write_DATA_VL(0x01);
		UTFT_LCD_Write_DATA_VL(0x00);//NW
		UTFT_LCD_Write_DATA_VL(0x43);

		UTFT_LCD_Write_COM(0xC1);
		UTFT_LCD_Write_DATA_VL(0x08);
		UTFT_LCD_Write_DATA_VL(0x15);//CLOCK
		UTFT_LCD_Write_DATA_VL(0x08);
		UTFT_LCD_Write_DATA_VL(0x08);

		UTFT_LCD_Write_COM(0xC4);
		UTFT_LCD_Write_DATA_VL(0x15);
		UTFT_LCD_Write_DATA_VL(0x03);
		UTFT_LCD_Write_DATA_VL(0x03);
		UTFT_LCD_Write_DATA_VL(0x01);

		UTFT_LCD_Write_COM(0xC6);
		UTFT_LCD_Write_DATA_VL(0x02);

		UTFT_LCD_Write_COM(0xC8);
		UTFT_LCD_Write_DATA_VL(0x0c);
		UTFT_LCD_Write_DATA_VL(0x05);
		UTFT_LCD_Write_DATA_VL(0x0A);//0X12
		UTFT_LCD_Write_DATA_VL(0x6B);//0x7D
		UTFT_LCD_Write_DATA_VL(0x04);
		UTFT_LCD_Write_DATA_VL(0x06);//0x08
		UTFT_LCD_Write_DATA_VL(0x15);//0x0A
		UTFT_LCD_Write_DATA_VL(0x10);
		UTFT_LCD_Write_DATA_VL(0x00);
		UTFT_LCD_Write_DATA_VL(0x60);//0x23
 
		UTFT_LCD_Write_COM(0x36);
		UTFT_LCD_Write_DATA_VL(0x0A);

		UTFT_LCD_Write_COM(0x0C);
		UTFT_LCD_Write_DATA_VL(0x55);

		UTFT_LCD_Write_COM(0x3A);
		UTFT_LCD_Write_DATA_VL(0x55);

		UTFT_LCD_Write_COM(0x38);
 
		UTFT_LCD_Write_COM(0xD0);
		UTFT_LCD_Write_DATA_VL(0x07);
		UTFT_LCD_Write_DATA_VL(0x07);//VCI1
		UTFT_LCD_Write_DATA_VL(0x14);//VRH 0x1D
		UTFT_LCD_Write_DATA_VL(0xA2);//BT 0x06

		UTFT_LCD_Write_COM(0xD1);
		UTFT_LCD_Write_DATA_VL(0x03);
		UTFT_LCD_Write_DATA_VL(0x5A);//VCM  0x5A
		UTFT_LCD_Write_DATA_VL(0x10);//VDV

		UTFT_LCD_Write_COM(0xD2);
		UTFT_LCD_Write_DATA_VL(0x03);
		UTFT_LCD_Write_DATA_VL(0x04);//0x24
		UTFT_LCD_Write_DATA_VL(0x04);

		UTFT_LCD_Write_COM(0x11);
		delay(150);

		UTFT_LCD_Write_COM(0x2A);
		UTFT_LCD_Write_DATA_VL(0x00);
		UTFT_LCD_Write_DATA_VL(0x00);
		UTFT_LCD_Write_DATA_VL(0x01);
		UTFT_LCD_Write_DATA_VL(0xDF);//320

		UTFT_LCD_Write_COM(0x2B);
		UTFT_LCD_Write_DATA_VL(0x00);
		UTFT_LCD_Write_DATA_VL(0x00);
		UTFT_LCD_Write_DATA_VL(0x01);
		UTFT_LCD_Write_DATA_VL(0x3F);//480

 
		delay(100);

		UTFT_LCD_Write_COM(0x29);
		delay(30);

		UTFT_LCD_Write_COM(0x2C);
		delay(30);
		break;
