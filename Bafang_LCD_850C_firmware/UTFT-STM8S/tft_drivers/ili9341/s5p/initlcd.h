case ILI9341_S5P:
    UTFT_LCD_Write_COM(0xCB);  
    UTFT_LCD_Write_DATA_VL(0x39); 
    UTFT_LCD_Write_DATA_VL(0x2C); 
    UTFT_LCD_Write_DATA_VL(0x00); 
    UTFT_LCD_Write_DATA_VL(0x34); 
    UTFT_LCD_Write_DATA_VL(0x02); 

    UTFT_LCD_Write_COM(0xCF);  
    UTFT_LCD_Write_DATA_VL(0x00); 
    UTFT_LCD_Write_DATA_VL(0XC1); 
    UTFT_LCD_Write_DATA_VL(0X30); 

    UTFT_LCD_Write_COM(0xE8);  
    UTFT_LCD_Write_DATA_VL(0x85); 
    UTFT_LCD_Write_DATA_VL(0x00); 
    UTFT_LCD_Write_DATA_VL(0x78); 

    UTFT_LCD_Write_COM(0xEA);  
    UTFT_LCD_Write_DATA_VL(0x00); 
    UTFT_LCD_Write_DATA_VL(0x00); 
 
    UTFT_LCD_Write_COM(0xED);  
    UTFT_LCD_Write_DATA_VL(0x64); 
    UTFT_LCD_Write_DATA_VL(0x03); 
    UTFT_LCD_Write_DATA_VL(0X12); 
    UTFT_LCD_Write_DATA_VL(0X81); 

    UTFT_LCD_Write_COM(0xF7);  
    UTFT_LCD_Write_DATA_VL(0x20); 
  
    UTFT_LCD_Write_COM(0xC0);    //Power control 
    UTFT_LCD_Write_DATA_VL(0x23);   //VRH[5:0] 
 
    UTFT_LCD_Write_COM(0xC1);    //Power control 
    UTFT_LCD_Write_DATA_VL(0x10);   //SAP[2:0];BT[3:0] 

    UTFT_LCD_Write_COM(0xC5);    //VCM control 
    UTFT_LCD_Write_DATA_VL(0x3e);   //Contrast
    UTFT_LCD_Write_DATA_VL(0x28); 
 
    UTFT_LCD_Write_COM(0xC7);    //VCM control2 
    UTFT_LCD_Write_DATA_VL(0x86);   //--
 
    UTFT_LCD_Write_COM(0x36);    // Memory Access Control 
    UTFT_LCD_Write_DATA_VL(0x48);   

    UTFT_LCD_Write_COM(0x3A);    
    UTFT_LCD_Write_DATA_VL(0x55); 

    UTFT_LCD_Write_COM(0xB1);    
    UTFT_LCD_Write_DATA_VL(0x00);  
    UTFT_LCD_Write_DATA_VL(0x18); 
 
    UTFT_LCD_Write_COM(0xB6);    // Display Function Control 
    UTFT_LCD_Write_DATA_VL(0x08); 
    UTFT_LCD_Write_DATA_VL(0x82);
    UTFT_LCD_Write_DATA_VL(0x27);  
/* 
    UTFT_LCD_Write_COM(0xF2);    // 3Gamma Function Disable 
    UTFT_LCD_Write_DATA_VL(0x00); 
 
    UTFT_LCD_Write_COM(0x26);    //Gamma curve selected 
    UTFT_LCD_Write_DATA_VL(0x01); 

    UTFT_LCD_Write_COM(0xE0);    //Set Gamma 
    UTFT_LCD_Write_DATA_VL(0x0F); 
    UTFT_LCD_Write_DATA_VL(0x31); 
    UTFT_LCD_Write_DATA_VL(0x2B); 
    UTFT_LCD_Write_DATA_VL(0x0C); 
    UTFT_LCD_Write_DATA_VL(0x0E); 
    UTFT_LCD_Write_DATA_VL(0x08); 
    UTFT_LCD_Write_DATA_VL(0x4E); 
    UTFT_LCD_Write_DATA_VL(0xF1); 
    UTFT_LCD_Write_DATA_VL(0x37); 
    UTFT_LCD_Write_DATA_VL(0x07); 
    UTFT_LCD_Write_DATA_VL(0x10); 
    UTFT_LCD_Write_DATA_VL(0x03); 
    UTFT_LCD_Write_DATA_VL(0x0E); 
    UTFT_LCD_Write_DATA_VL(0x09); 
    UTFT_LCD_Write_DATA_VL(0x00); 

    UTFT_LCD_Write_COM(0XE1);    //Set Gamma 
    UTFT_LCD_Write_DATA_VL(0x00); 
    UTFT_LCD_Write_DATA_VL(0x0E); 
    UTFT_LCD_Write_DATA_VL(0x14); 
    UTFT_LCD_Write_DATA_VL(0x03); 
    UTFT_LCD_Write_DATA_VL(0x11); 
    UTFT_LCD_Write_DATA_VL(0x07); 
    UTFT_LCD_Write_DATA_VL(0x31); 
    UTFT_LCD_Write_DATA_VL(0xC1); 
    UTFT_LCD_Write_DATA_VL(0x48); 
    UTFT_LCD_Write_DATA_VL(0x08); 
    UTFT_LCD_Write_DATA_VL(0x0F); 
    UTFT_LCD_Write_DATA_VL(0x0C); 
    UTFT_LCD_Write_DATA_VL(0x31); 
    UTFT_LCD_Write_DATA_VL(0x36); 
    UTFT_LCD_Write_DATA_VL(0x0F); 
*/
    UTFT_LCD_Write_COM(0x11);    //Exit Sleep 
    delay(120); 
				
    UTFT_LCD_Write_COM(0x29);    //Display on 
    UTFT_LCD_Write_COM(0x2c); 
	break;
