case S1D19122:
	//************* Start Initial Sequence **********//
	{
	int i,R,G,B;
	UTFT_LCD_Write_COM(0x11);
	UTFT_LCD_Write_COM(0x13);
	UTFT_LCD_Write_COM(0x29);
    
	//--------------  Display Control ---------//
	UTFT_LCD_Write_COM(0xB0);

	UTFT_LCD_Write_DATA_VL(0x05);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0xF0);
	UTFT_LCD_Write_DATA_VL(0x0A);
	UTFT_LCD_Write_DATA_VL(0x41);
	UTFT_LCD_Write_DATA_VL(0x02); 
	UTFT_LCD_Write_DATA_VL(0x0A);
	UTFT_LCD_Write_DATA_VL(0x30);
	UTFT_LCD_Write_DATA_VL(0x31);
	UTFT_LCD_Write_DATA_VL(0x36);
	UTFT_LCD_Write_DATA_VL(0x37);
	UTFT_LCD_Write_DATA_VL(0x40);
	UTFT_LCD_Write_DATA_VL(0x02);
	UTFT_LCD_Write_DATA_VL(0x3F);
	UTFT_LCD_Write_DATA_VL(0x40);
	UTFT_LCD_Write_DATA_VL(0x02);
	UTFT_LCD_Write_DATA_VL(0x81);
	UTFT_LCD_Write_DATA_VL(0x04);
	UTFT_LCD_Write_DATA_VL(0x05);
	UTFT_LCD_Write_DATA_VL(0x64);

	// ----------- Gamma  Curve  Set3 Postive----------//
	UTFT_LCD_Write_COM(0xFC);

	UTFT_LCD_Write_DATA_VL(0x88);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x10);
	UTFT_LCD_Write_DATA_VL(0x01);
	UTFT_LCD_Write_DATA_VL(0x01);
	UTFT_LCD_Write_DATA_VL(0x10);
	UTFT_LCD_Write_DATA_VL(0x42);
	UTFT_LCD_Write_DATA_VL(0x42);
	UTFT_LCD_Write_DATA_VL(0x22);
	UTFT_LCD_Write_DATA_VL(0x11);
	UTFT_LCD_Write_DATA_VL(0x11);
	UTFT_LCD_Write_DATA_VL(0x22);
	UTFT_LCD_Write_DATA_VL(0x99);
	UTFT_LCD_Write_DATA_VL(0xAA);
	UTFT_LCD_Write_DATA_VL(0xAA);
	UTFT_LCD_Write_DATA_VL(0xAA);
	UTFT_LCD_Write_DATA_VL(0xBB);
	UTFT_LCD_Write_DATA_VL(0xBB);
	UTFT_LCD_Write_DATA_VL(0xAA);
	UTFT_LCD_Write_DATA_VL(0x33);
	UTFT_LCD_Write_DATA_VL(0x33);
	UTFT_LCD_Write_DATA_VL(0x11);
	UTFT_LCD_Write_DATA_VL(0x01);
	UTFT_LCD_Write_DATA_VL(0x01);
	UTFT_LCD_Write_DATA_VL(0x01);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0xC0);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x00);

	// ----------- Gamma  Curve  Set3 Negative----------//
	UTFT_LCD_Write_COM(0xFD);

	UTFT_LCD_Write_DATA_VL(0x88);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x10);
	UTFT_LCD_Write_DATA_VL(0x01);
	UTFT_LCD_Write_DATA_VL(0x01);
	UTFT_LCD_Write_DATA_VL(0x10);
	UTFT_LCD_Write_DATA_VL(0x42);
	UTFT_LCD_Write_DATA_VL(0x42);
	UTFT_LCD_Write_DATA_VL(0x22);
	UTFT_LCD_Write_DATA_VL(0x11);
	UTFT_LCD_Write_DATA_VL(0x11);
	UTFT_LCD_Write_DATA_VL(0x22);
	UTFT_LCD_Write_DATA_VL(0x99);
	UTFT_LCD_Write_DATA_VL(0xAA);
	UTFT_LCD_Write_DATA_VL(0xAA);
	UTFT_LCD_Write_DATA_VL(0xAA);
	UTFT_LCD_Write_DATA_VL(0xBB);
	UTFT_LCD_Write_DATA_VL(0xBB);
	UTFT_LCD_Write_DATA_VL(0xAA);
	UTFT_LCD_Write_DATA_VL(0x33);
	UTFT_LCD_Write_DATA_VL(0x33);
	UTFT_LCD_Write_DATA_VL(0x11);
	UTFT_LCD_Write_DATA_VL(0x01);
	UTFT_LCD_Write_DATA_VL(0x01);
	UTFT_LCD_Write_DATA_VL(0x01);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x03);

	// ----------- EVRSER Regulator Voltage Setting---------//
	UTFT_LCD_Write_COM(0xBE);

	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x15);
	UTFT_LCD_Write_DATA_VL(0x16);
	UTFT_LCD_Write_DATA_VL(0x08);
	UTFT_LCD_Write_DATA_VL(0x09);
	UTFT_LCD_Write_DATA_VL(0x15);
	UTFT_LCD_Write_DATA_VL(0x10);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x00);

	// -----------Module Definiton Setting---------//
	UTFT_LCD_Write_COM(0xC0);

	UTFT_LCD_Write_DATA_VL(0x0E);
	UTFT_LCD_Write_DATA_VL(0x01);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_DATA_VL(0x00);

	// -----------PWRDEF Power Ability Ddfinition----------//
	UTFT_LCD_Write_COM(0xC1);

	UTFT_LCD_Write_DATA_VL(0x2F);
	UTFT_LCD_Write_DATA_VL(0x23);
	UTFT_LCD_Write_DATA_VL(0xB4);
	UTFT_LCD_Write_DATA_VL(0xFF);
	UTFT_LCD_Write_DATA_VL(0x24);
	UTFT_LCD_Write_DATA_VL(0x03);
	UTFT_LCD_Write_DATA_VL(0x20);
	UTFT_LCD_Write_DATA_VL(0x02);
	UTFT_LCD_Write_DATA_VL(0x02);
	UTFT_LCD_Write_DATA_VL(0x02);
	UTFT_LCD_Write_DATA_VL(0x20);
	UTFT_LCD_Write_DATA_VL(0x20);
	UTFT_LCD_Write_DATA_VL(0x00);

	// -----------Other Setting----------//
	UTFT_LCD_Write_COM(0xC2);
	UTFT_LCD_Write_DATA_VL(0x03);
	UTFT_LCD_Write_COM(0x26);
	UTFT_LCD_Write_DATA_VL(0x08);
	UTFT_LCD_Write_COM(0x35);
   
	UTFT_LCD_Write_COM(0x36);
	UTFT_LCD_Write_DATA_VL(0x64);
	UTFT_LCD_Write_COM(0x3A);
	UTFT_LCD_Write_DATA_VL(0x05);
	UTFT_LCD_Write_COM(0x2A);
	UTFT_LCD_Write_DATA(0x01,0x3f);
	UTFT_LCD_Write_COM(0x2B);
	UTFT_LCD_Write_DATA_VL(0xEF);
	UTFT_LCD_Write_COM(0x2c);

	// -----------RGB Setting----------//
	UTFT_LCD_Write_COM(0x2D);
	R=0;
	G=0;
	B=0;   
    
	for(i=0;i<32;i++)
	{ 
		UTFT_LCD_Write_DATA_VL(R);
		R=R+2;
	}
	for(i=0;i<64;i++)
	{ 
		UTFT_LCD_Write_DATA_VL(G);
		G=G+1;
	} 
	for(i=0;i<32;i++)
	{ 
		UTFT_LCD_Write_DATA_VL(B);
		B=B+2;
	}
	}
	break;
