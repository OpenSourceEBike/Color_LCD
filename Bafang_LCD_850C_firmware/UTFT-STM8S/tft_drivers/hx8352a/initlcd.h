case HX8352A:
	UTFT_LCD_Write_COM(0x83);           
	UTFT_LCD_Write_DATA_VL(0x02);  //TESTM=1 
             
	UTFT_LCD_Write_COM(0x85);  
	UTFT_LCD_Write_DATA_VL(0x03);  //VDC_SEL=011
	UTFT_LCD_Write_COM(0x8B);  
	UTFT_LCD_Write_DATA_VL(0x01);
	UTFT_LCD_Write_COM(0x8C);  
	UTFT_LCD_Write_DATA_VL(0x93); //STBA[7]=1,STBA[5:4]=01,STBA[1:0]=11
        
	UTFT_LCD_Write_COM(0x91);  
	UTFT_LCD_Write_DATA_VL(0x01); //DCDC_SYNC=1
        
	UTFT_LCD_Write_COM(0x83);  
	UTFT_LCD_Write_DATA_VL(0x00); //TESTM=0
	//Gamma Setting

	UTFT_LCD_Write_COM(0x3E);  
	UTFT_LCD_Write_DATA_VL(0xB0);
	UTFT_LCD_Write_COM(0x3F);  
	UTFT_LCD_Write_DATA_VL(0x03);
	UTFT_LCD_Write_COM(0x40);  
	UTFT_LCD_Write_DATA_VL(0x10);
	UTFT_LCD_Write_COM(0x41);  
	UTFT_LCD_Write_DATA_VL(0x56);
	UTFT_LCD_Write_COM(0x42);  
	UTFT_LCD_Write_DATA_VL(0x13);
	UTFT_LCD_Write_COM(0x43);  
	UTFT_LCD_Write_DATA_VL(0x46);
	UTFT_LCD_Write_COM(0x44);  
	UTFT_LCD_Write_DATA_VL(0x23);
	UTFT_LCD_Write_COM(0x45);  
	UTFT_LCD_Write_DATA_VL(0x76);
	UTFT_LCD_Write_COM(0x46);  
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_COM(0x47);  
	UTFT_LCD_Write_DATA_VL(0x5E);
	UTFT_LCD_Write_COM(0x48);  
	UTFT_LCD_Write_DATA_VL(0x4F);
	UTFT_LCD_Write_COM(0x49);  
	UTFT_LCD_Write_DATA_VL(0x40);	
	//**********Power On sequence************
        
	UTFT_LCD_Write_COM(0x17);  
	UTFT_LCD_Write_DATA_VL(0x91);
       
	UTFT_LCD_Write_COM(0x2B);  
	UTFT_LCD_Write_DATA_VL(0xF9);
	delay(10);
        
	UTFT_LCD_Write_COM(0x1B);  
	UTFT_LCD_Write_DATA_VL(0x14);
        
	UTFT_LCD_Write_COM(0x1A);  
	UTFT_LCD_Write_DATA_VL(0x11);
              
	UTFT_LCD_Write_COM(0x1C);  
	UTFT_LCD_Write_DATA_VL(0x06);
        
	UTFT_LCD_Write_COM(0x1F);  
	UTFT_LCD_Write_DATA_VL(0x42);
	delay(20);
        
	UTFT_LCD_Write_COM(0x19);  
	UTFT_LCD_Write_DATA_VL(0x0A);
     
	UTFT_LCD_Write_COM(0x19);  
	UTFT_LCD_Write_DATA_VL(0x1A);
	delay(40);
        
        
	UTFT_LCD_Write_COM(0x19);  
	UTFT_LCD_Write_DATA_VL(0x12);
	delay(40);
        
	UTFT_LCD_Write_COM(0x1E);  
	UTFT_LCD_Write_DATA_VL(0x27);
	delay(100);	   
        
        
	//**********DISPLAY ON SETTING***********
        
	UTFT_LCD_Write_COM(0x24);  
	UTFT_LCD_Write_DATA_VL(0x60);
        
	UTFT_LCD_Write_COM(0x3D);  
	UTFT_LCD_Write_DATA_VL(0x40);
        
	UTFT_LCD_Write_COM(0x34);  
	UTFT_LCD_Write_DATA_VL(0x38);
        
	UTFT_LCD_Write_COM(0x35);  
	UTFT_LCD_Write_DATA_VL(0x38);
        
	UTFT_LCD_Write_COM(0x24);  
	UTFT_LCD_Write_DATA_VL(0x38);
	delay(40);
        
	UTFT_LCD_Write_COM(0x24);  
	UTFT_LCD_Write_DATA_VL(0x3C);
        
	UTFT_LCD_Write_COM(0x16);  
	UTFT_LCD_Write_DATA_VL(0x1C);
        
	UTFT_LCD_Write_COM(0x01);  
	UTFT_LCD_Write_DATA_VL(0x06);
        
	UTFT_LCD_Write_COM(0x55);  
	UTFT_LCD_Write_DATA_VL(0x00); 

	UTFT_LCD_Write_COM(0x02);           
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_COM(0x03);           
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_COM(0x04);           
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_COM(0x05);           
	UTFT_LCD_Write_DATA_VL(0xef);
        
	UTFT_LCD_Write_COM(0x06);           
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_COM(0x07);           
	UTFT_LCD_Write_DATA_VL(0x00);
	UTFT_LCD_Write_COM(0x08);           
	UTFT_LCD_Write_DATA_VL(0x01);
	UTFT_LCD_Write_COM(0x09);           
	UTFT_LCD_Write_DATA_VL(0x8f);

	UTFT_LCD_Write_COM(0x22);
	break;
