case HX8340B_8:
	UTFT_LCD_Write_COM_DATA(0x26,0x0084); //PT=10,GON=0, DTE=0, D=0100
	delay(40);
	UTFT_LCD_Write_COM_DATA(0x26,0x00B8); //PT=10,GON=1, DTE=1, D=1000
	delay(40);
	UTFT_LCD_Write_COM_DATA(0x26,0x00BC); //PT=10,GON=1, DTE=1, D=1100
	delay(20);
	// UTFT_LCD_Write_COM_DATA(0x0001,0x0000);     // PTL='1' Enter Partail mode

	//Driving ability Setting
	UTFT_LCD_Write_COM_DATA(0x60,0x0000);
	UTFT_LCD_Write_COM_DATA(0x61,0x0006);
	UTFT_LCD_Write_COM_DATA(0x62,0x0000);
	UTFT_LCD_Write_COM_DATA(0x63,0x00C8);
	delay(20);

	//Gamma Setting
	UTFT_LCD_Write_COM_DATA(0x73,0x0070);
	UTFT_LCD_Write_COM_DATA(0x40,0x0000);
	UTFT_LCD_Write_COM_DATA(0x41,0x0040);
	UTFT_LCD_Write_COM_DATA(0x42,0x0045);
	UTFT_LCD_Write_COM_DATA(0x43,0x0001);
	UTFT_LCD_Write_COM_DATA(0x44,0x0060);
	UTFT_LCD_Write_COM_DATA(0x45,0x0005);
	UTFT_LCD_Write_COM_DATA(0x46,0x000C);
	UTFT_LCD_Write_COM_DATA(0x47,0x00D1);
	UTFT_LCD_Write_COM_DATA(0x48,0x0005);

	UTFT_LCD_Write_COM_DATA(0x50,0x0075);
	UTFT_LCD_Write_COM_DATA(0x51,0x0001);
	UTFT_LCD_Write_COM_DATA(0x52,0x0067);
	UTFT_LCD_Write_COM_DATA(0x53,0x0014);
	UTFT_LCD_Write_COM_DATA(0x54,0x00F2);
	UTFT_LCD_Write_COM_DATA(0x55,0x0007);
	UTFT_LCD_Write_COM_DATA(0x56,0x0003);
	UTFT_LCD_Write_COM_DATA(0x57,0x0049);
	delay(20);

	//Power Setting
	UTFT_LCD_Write_COM_DATA(0x1F,0x0003); //VRH=4.65V     VREG1?GAMMA? 00~1E  080421    
	UTFT_LCD_Write_COM_DATA(0x20,0x0000); //BT (VGH~15V,VGL~-12V,DDVDH~5V)
	UTFT_LCD_Write_COM_DATA(0x24,0x0024); //VCOMH(VCOM High voltage3.2V)     0024/12    080421    11~40
	UTFT_LCD_Write_COM_DATA(0x25,0x0034); //VCOML(VCOM Low voltage -1.2V)    0034/4A    080421    29~3F 
	//****VCOM offset**///
	UTFT_LCD_Write_COM_DATA(0x23,0x002F); //VMF(no offset)                            
	delay(20);

	//##################################################################
	// Power Supply Setting
	UTFT_LCD_Write_COM_DATA(0x18,0x0044); //I/P_RADJ,N/P_RADJ Noraml mode 60Hz
	UTFT_LCD_Write_COM_DATA(0x21,0x0001); //OSC_EN='1' start osc
	UTFT_LCD_Write_COM_DATA(0x01,0x0000); //SLP='0' out sleep
	UTFT_LCD_Write_COM_DATA(0x1C,0x0003); //AP=011
	UTFT_LCD_Write_COM_DATA(0x19,0x0006); // VOMG=1,PON=1, DK=0,
	delay(20);

	//##################################################################
	// Display ON Setting
	UTFT_LCD_Write_COM_DATA(0x26,0x0084); //PT=10,GON=0, DTE=0, D=0100
	delay(40);
	UTFT_LCD_Write_COM_DATA(0x26,0x00B8); //PT=10,GON=1, DTE=1, D=1000
	delay(40);
	UTFT_LCD_Write_COM_DATA(0x26,0x00BC); //PT=10,GON=1, DTE=1, D=1100
	delay(20);

	//SET GRAM AREA
	UTFT_LCD_Write_COM_DATA(0x02,0x0000); 
	UTFT_LCD_Write_COM_DATA(0x03,0x0000); 
	UTFT_LCD_Write_COM_DATA(0x04,0x0000);
	UTFT_LCD_Write_COM_DATA(0x05,0x00AF);
	UTFT_LCD_Write_COM_DATA(0x06,0x0000);
	UTFT_LCD_Write_COM_DATA(0x07,0x0000);
	UTFT_LCD_Write_COM_DATA(0x08,0x0000);
	UTFT_LCD_Write_COM_DATA(0x09,0x00DB);
	delay(20);
	UTFT_LCD_Write_COM_DATA(0x16,0x0008);  //MV MX MY ML SET  0028
	UTFT_LCD_Write_COM_DATA(0x17,0x0005);//COLMOD Control Register (R17h)
	UTFT_LCD_Write_COM(0x21);
	UTFT_LCD_Write_COM(0x22);
	break;
