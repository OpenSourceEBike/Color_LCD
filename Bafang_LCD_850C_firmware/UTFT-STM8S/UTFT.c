/*
  UTFT.cpp - Multi-Platform library support for Color TFT LCD Boards
  Copyright (C)2015-2018 Rinky-Dink Electronics, Henning Karlsen. All right reserved
  
  This library is the continuation of my ITDB02_Graph, ITDB02_Graph16
  and RGB_GLCD libraries for Arduino and chipKit. As the number of 
  supported display modules and controllers started to increase I felt 
  it was time to make a single, universal library as it will be much 
  easier to maintain in the future.

  Basic functionality of this library was origianlly based on the 
  demo-code provided by ITead studio (for the ITDB02 modules) and 
  NKC Electronics (for the RGB GLCD module/shield).

  This library supports a number of 8bit, 16bit and serial graphic 
  displays, and will work with both Arduino, chipKit boards and select 
  TI LaunchPads. For a full list of tested display modules and controllers,
  see the document UTFT_Supported_display_modules_&_controllers.pdf.

  When using 8bit and 16bit display modules there are some 
  requirements you must adhere to. These requirements can be found 
  in the document UTFT_Requirements.pdf.
  There are no special requirements when using serial displays.

  You can find the latest version of the library at 
  http://www.RinkyDinkElectronics.com/

  This library is free software; you can redistribute it and/or
  modify it under the terms of the CC BY-NC-SA 3.0 license.
  Please see the included documents for further information.

  Commercial use of this library requires you to buy a license that
  will allow commercial use. This includes using the library,
  modified or not, as a tool to sell products.

  The license applies to all part of the library including the 
  examples and tools supplied with the library.
*/

#include <string.h>
#include "UTFT.h"
#include "hardware/stm32f103/hw_stm32f103_defines.h"
#include "hardware/stm32f103/hw_bafang_850c.h"
#include "pins.h"
#include "delay.h"

// Include hardware-specific functions for the correct MCU
#include "hardware/stm8/HW_STM8.h"
#include "memorysaver.h"

// stuff copied from UTFT.h because of sdcc error "Multiple definition of"
// https://sourceforge.net/p/sdcc/discussion/1865/thread/bd9a4dfc/
byte			UTFT_fch, UTFT_fcl, UTFT_bch, UTFT_bcl;
byte			UTFT_orient;
long			UTFT_disp_x_size, UTFT_disp_y_size;
byte			UTFT_display_model, UTFT_display_transfer_mode, UTFT_display_serial_mode;
//regtype			*UTFT_P_RS, *UTFT_P_WR, *UTFT_P_CS, *UTFT_P_RST, *UTFT_P_SDA, *UTFT_P_SCL, *UTFT_P_ALE;
regtype     *UTFT_P_SDA, *UTFT_P_SCL;
GPIO_TypeDef  *UTFT_P_RS, *UTFT_P_WR, *UTFT_P_CS, *UTFT_P_RST, *UTFT_P_ALE;

//regsize			UTFT_B_RS, UTFT_B_WR, UTFT_B_CS, UTFT_B_RST, UTFT_B_SDA, UTFT_B_SCL, UTFT_B_ALE;
regsize      UTFT_B_SDA, UTFT_B_SCL;
uint16_t     UTFT_B_RS, UTFT_B_WR, UTFT_B_CS, UTFT_B_RST, UTFT_B_ALE;

struct _current_font	UTFT_cfont;
boolean			UTFT__transparent;

word			UTFT_offset_x, UTFT_offset_y;

void UTFT (void)
{
  UTFT_SER(ILI9481);
}

void UTFT_SER(byte model)
{ 
//	word	UTFT_dsx[33] = {239, 239, 239, 239, 239, 239, 175, 175, 239, 127,						// 00-09
//					 127, 239, 271, 479, 239, 239, 239, 239, 239, 239,						// 10-19
//					 479, 319, 239, 175,   0, 239, 239, 319, 319, 799,						// 20-29
//					 127, 127, 239};																	// 30-32
//	word	UTFT_dsy[33] = {319, 399, 319, 319, 319, 319, 219, 219, 399, 159,						// 00-09
//					 127, 319, 479, 799, 319, 319, 319, 319, 319, 319,						// 10-19
//					 799, 479, 319, 219,   0, 319, 319, 479, 479, 479,						// 20-29
//					 159, 159, 319};																	// 30-32
//	byte	UTFT_dtm[33] = {16, 16, 16, 8, 8, 16, 8, SERIAL_4PIN, 16, SERIAL_5PIN,					// 00-09
//					 SERIAL_5PIN, 16, 16, 16, 8, 16, LATCHED_16, 16, 8, 8,					// 10-19
//					 16, 16, 16, 8, 0, SERIAL_5PIN, SERIAL_4PIN, 16, 16, 16,				// 20-29
//					 SERIAL_5PIN, 8, 16};															// 30-32

//	UTFT_disp_x_size =			UTFT_dsx[model];
//	UTFT_disp_y_size =			UTFT_dsy[model];
//	UTFT_display_transfer_mode =	UTFT_dtm[model];
//	UTFT_display_model =			model;

  UTFT_disp_x_size =      319;
  UTFT_disp_y_size =      479;
  UTFT_display_transfer_mode =  16;
  UTFT_display_model =      ILI9481;

//	UTFT___p1 = RS;
//	UTFT___p2 = WR;
//	UTFT___p3 = CS;
//	UTFT___p4 = _RST;
//	UTFT___p5 = SER;

	if (UTFT_display_transfer_mode == SERIAL_4PIN)
	{
		UTFT_display_transfer_mode=1;
		UTFT_display_serial_mode=SERIAL_4PIN;
	}
	if (UTFT_display_transfer_mode == SERIAL_5PIN)
	{
		UTFT_display_transfer_mode=1;
		UTFT_display_serial_mode=SERIAL_5PIN;
	}

	if (UTFT_display_transfer_mode!=1)
	{
		UTFT__set_direction_registers(UTFT_display_transfer_mode);

//		UTFT_P_RS	= portOutputRegister(digitalPinToPort(RS));
//		UTFT_B_RS	= digitalPinToBitMask(RS);
    UTFT_P_RS = LCD_COMMAND_DATA__PORT;
    UTFT_B_RS = LCD_COMMAND_DATA__PIN;

//		UTFT_P_WR	= portOutputRegister(digitalPinToPort(WR));
//		UTFT_B_WR	= digitalPinToBitMask(WR);
    UTFT_P_WR = LCD_WRITE__PORT;
    UTFT_B_WR = LCD_WRITE__PIN;

//		UTFT_P_CS	= portOutputRegister(digitalPinToPort(CS));
//		UTFT_B_CS	= digitalPinToBitMask(CS);
    UTFT_P_CS = LCD_CHIP_SELECT__PORT;
    UTFT_B_CS = LCD_CHIP_SELECT__PIN;

//		UTFT_P_RST	= portOutputRegister(digitalPinToPort(_RST));
//		UTFT_B_RST	= digitalPinToBitMask(_RST);

		if (UTFT_display_transfer_mode==LATCHED_16)
		{
//			UTFT_P_ALE	= portOutputRegister(digitalPinToPort(SER));
//			UTFT_B_ALE	= digitalPinToBitMask(SER);
//			cbi(UTFT_P_ALE, UTFT_B_ALE);
//			pinMode(8,OUTPUT);
//			digitalWrite(8, LOW);
		}
	}
	else
	{
//		UTFT_P_SDA	= portOutputRegister(digitalPinToPort(RS));
//		UTFT_B_SDA	= digitalPinToBitMask(RS);
//		UTFT_P_SCL	= portOutputRegister(digitalPinToPort(WR));
//		UTFT_B_SCL	= digitalPinToBitMask(WR);
//		UTFT_P_CS	= portOutputRegister(digitalPinToPort(CS));
//		UTFT_B_CS	= digitalPinToBitMask(CS);
//		if (RST != NOTINUSE)
//		{
//			UTFT_P_RST	= portOutputRegister(digitalPinToPort(_RST));
//			UTFT_B_RST	= digitalPinToBitMask(_RST);
//		}
//		if (UTFT_display_serial_mode!=SERIAL_4PIN)
//		{
//			UTFT_P_RS	= portOutputRegister(digitalPinToPort(SER));
//			UTFT_B_RS	= digitalPinToBitMask(SER);
//		}
	}
}

uint32_t UTFT_read_reg_0 (uint8_t ui8_reg)
{
  static uint16_t ui16_i;
  uint32_t ui32_reg_0_value = 0;
  uint32_t ui32_reg_0_value_array [2];
  
  ui16_i = 0;

  cbi(UTFT_P_CS, UTFT_B_CS);
//  UTFT_LCD_Write_COM (ui8_reg);
//  UTFT_LCD_Write_Bus(146, 34, 16); // 146: driver ID ILI9320 --> 0X9232
//  UTFT_LCD_Write_Bus(147, 37, 16); // 9325 --> OUTPUT: 0X9335
  cbi(UTFT_P_RS, UTFT_B_RS);
//  UTFT_LCD_Write_Bus(147, 53, 16); // 9335 --> OUTPUT: 0X9335

  UTFT_LCD_Write_Bus(93, 26, 16); // 9335 --> OUTPUT:

  sbi(UTFT_P_CS, UTFT_B_CS);
  
  // set data lines as input
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = 0xffff;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  delay (10);
  
  cbi(UTFT_P_CS, UTFT_B_CS);
  for (ui16_i = 0; ui16_i < 1; ui16_i++)
  {
    GPIO_SetBits(LCD_READ__PORT, LCD_READ__PIN);
    sbi(UTFT_P_RS, UTFT_B_RS); // data on BUS is data
    sbi(UTFT_P_WR, UTFT_B_WR);

    GPIO_ResetBits(LCD_READ__PORT, LCD_READ__PIN);
    
    ui32_reg_0_value = (uint32_t) (GPIO_ReadInputData (GPIOB));

    GPIO_SetBits(LCD_READ__PORT, LCD_READ__PIN);

    if ((ui32_reg_0_value != 0) && (ui32_reg_0_value != 0xffff))
      {
        ui32_reg_0_value_array [ui16_i] = 1;
      }
  }
  sbi(UTFT_P_CS, UTFT_B_CS);
  
  // set data lines as output again
  GPIO_InitStructure.GPIO_Pin = 0xffff;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  ui32_reg_0_value = ui32_reg_0_value_array [0] + (ui32_reg_0_value_array [1] << 16);
  return ui32_reg_0_value;
}

void UTFT_LCD_Write_COM(char VL)  
{   
	if (UTFT_display_transfer_mode!=1)
	{
	  // command mode
    GPIO_ResetBits(LCD_COMMAND_DATA__PORT, LCD_COMMAND_DATA__PIN);

		UTFT_LCD_Write_Bus(0x00,VL,UTFT_display_transfer_mode);
	}
	else
		UTFT_LCD_Write_Bus(0x00,VL,UTFT_display_transfer_mode);
}

void UTFT_LCD_Write_COM_16b(uint16_t ui16_command)
{
    cbi(UTFT_P_RS, UTFT_B_RS);
    UTFT_LCD_Write_Bus((char) ((ui16_command >> 8) & 0xff), (char) (ui16_command & 0xff), UTFT_display_transfer_mode);
}

void UTFT_LCD_Write_DATA(char VH,char VL)
{
	if (UTFT_display_transfer_mode!=1)
	{
		sbi(UTFT_P_RS, UTFT_B_RS);
		UTFT_LCD_Write_Bus(VH,VL,UTFT_display_transfer_mode);
	}
	else
	{
		UTFT_LCD_Write_Bus(0x01,VH,UTFT_display_transfer_mode);
		UTFT_LCD_Write_Bus(0x01,VL,UTFT_display_transfer_mode);
	}
}

void UTFT_LCD_Write_DATA_VL(char VL)
{
	if (UTFT_display_transfer_mode!=1)
	{
    // data mode
    GPIO_SetBits(LCD_COMMAND_DATA__PORT, LCD_COMMAND_DATA__PIN);

		UTFT_LCD_Write_Bus(0x00,VL,UTFT_display_transfer_mode);
	}
	else
		UTFT_LCD_Write_Bus(0x01,VL,UTFT_display_transfer_mode);
}

void UTFT_LCD_Write_COM_DATA(char com1,int dat1)
{
     UTFT_LCD_Write_COM(com1);
     UTFT_LCD_Write_DATA(dat1>>8,dat1);
}

void UTFT_LCD_Write_DATA_8(char VL)
{	
    sbi(UTFT_P_RS, UTFT_B_RS);
    UTFT_LCD_Write_Bus_8(VL);
}

void UTFT_InitLCD()
{
  UTFT_InitLCD_orientation(LANDSCAPE);
}

void UTFT_InitLCD_orientation(byte orientation)
{
	UTFT_orient=orientation;
	UTFT__hw_special_init();

//	pinMode(UTFT___p1,OUTPUT);
//	pinMode(UTFT___p2,OUTPUT);
//	pinMode(UTFT___p3,OUTPUT);
//	if (UTFT___p4 != NOTINUSE)
//		pinMode(UTFT___p4,OUTPUT);
//	if ((UTFT_display_transfer_mode==LATCHED_16) || ((UTFT_display_transfer_mode==1) && (UTFT_display_serial_mode==SERIAL_5PIN)))
//		pinMode(UTFT___p5,OUTPUT);
	if (UTFT_display_transfer_mode!=1)
		UTFT__set_direction_registers(UTFT_display_transfer_mode);

	//sbi(UTFT_P_RST, UTFT_B_RST);
//	delay(5);
	//cbi(UTFT_P_RST, UTFT_B_RST);
//	delay(15);
	//sbi(UTFT_P_RST, UTFT_B_RST);
//	delay(15);

  // chip select active
  GPIO_ResetBits(LCD_CHIP_SELECT__PORT, LCD_CHIP_SELECT__PIN);

	switch(UTFT_display_model)
	{
#ifndef DISABLE_HX8347A
	#include "tft_drivers/hx8347a/initlcd.h"
#endif
#ifndef DISABLE_ILI9327
	#include "tft_drivers/ili9327/initlcd.h"
#endif
#ifndef DISABLE_SSD1289
	#include "tft_drivers/ssd1289/initlcd.h"
#endif
#ifndef DISABLE_ILI9325C
	#include "tft_drivers/ili9325c/initlcd.h"
#endif
#ifndef DISABLE_ILI9325D
	#include "tft_drivers/ili9325d/default/initlcd.h"
#endif
#ifndef DISABLE_ILI9325D_ALT
	#include "tft_drivers/ili9325d/alt/initlcd.h"
#endif
#ifndef DISABLE_HX8340B_8
	#include "tft_drivers/hx8340b/8/initlcd.h"
#endif
#ifndef DISABLE_HX8340B_S
	#include "tft_drivers/hx8340b/s/initlcd.h"
#endif
#ifndef DISABLE_ST7735
	#include "tft_drivers/st7735/std/initlcd.h"
#endif
#ifndef DISABLE_PCF8833
	#include "tft_drivers/pcf8833/initlcd.h"
#endif
#ifndef DISABLE_S1D19122
	#include "tft_drivers/s1d19122/initlcd.h"
#endif
#ifndef DISABLE_HX8352A
	#include "tft_drivers/hx8352a/initlcd.h"
#endif
#ifndef DISABLE_SSD1963_480
	#include "tft_drivers/ssd1963/480/initlcd.h"
#endif
#ifndef DISABLE_SSD1963_800
	#include "tft_drivers/ssd1963/800/initlcd.h"
#endif
#ifndef DISABLE_SSD1963_800_ALT
	#include "tft_drivers/ssd1963/800alt/initlcd.h"
#endif
#ifndef DISABLE_S6D1121
	#include "tft_drivers/s6d1121/initlcd.h"
#endif
#ifndef DISABLE_ILI9481
	#include "tft_drivers/ili9481/initlcd.h"
#endif
#ifndef DISABLE_S6D0164
	#include "tft_drivers/s6d0164/initlcd.h"
#endif
#ifndef DISABLE_ILI9341_S4P
	#include "tft_drivers/ili9341/s4p/initlcd.h"
#endif
#ifndef DISABLE_ILI9341_S5P
	#include "tft_drivers/ili9341/s5p/initlcd.h"
#endif
#ifndef DISABLE_ILI9341_8
	#include "tft_drivers/ili9341/8/initlcd.h"
#endif
#ifndef DISABLE_ILI9341_16
	#include "tft_drivers/ili9341/16/initlcd.h"
#endif
#ifndef DISABLE_R61581
	#include "tft_drivers/r61581/initlcd.h"
#endif
#ifndef DISABLE_ILI9486
	#include "tft_drivers/ili9486/initlcd.h"
#endif
#ifndef DISABLE_CPLD
	#include "tft_drivers/cpld/initlcd.h"
#endif
#ifndef DISABLE_HX8353C
	#include "tft_drivers/hx8353c/initlcd.h"
#endif
#ifndef DISABLE_ST7735_8
	#include "tft_drivers/st7735/8/initlcd.h"
#endif
#ifndef DISABLE_ILI9335
  #include "tft_drivers/ili9335/initlcd.h"
#endif
	}

  // chip select no active
  GPIO_SetBits(LCD_CHIP_SELECT__PORT, LCD_CHIP_SELECT__PIN);

	UTFT_setColor_rgb(255, 255, 255);
	UTFT_setBackColor_rgb(0, 0, 0);
	UTFT_cfont.font=0;
	UTFT__transparent = false;
}

void UTFT_setXY(word x1, word y1, word x2, word y2)
{
  x1 = x1 + UTFT_offset_x;
  x2 = x2 + UTFT_offset_x;
  y1 = y1 + UTFT_offset_y;
  y2 = y2 + UTFT_offset_y;
  
	if (UTFT_orient==LANDSCAPE)
	{
		swap(word, x1, y1);
		swap(word, x2, y2)
		y1=UTFT_disp_y_size-y1;
		y2=UTFT_disp_y_size-y2;
		swap(word, y1, y2)
	}

	switch(UTFT_display_model)
	{
#ifndef DISABLE_HX8347A
	#include "tft_drivers/hx8347a/setxy.h"
#endif
#ifndef DISABLE_HX8352A
	#include "tft_drivers/hx8352a/setxy.h"
#endif
#ifndef DISABLE_ILI9327
	#include "tft_drivers/ili9327/setxy.h"
#endif
#ifndef DISABLE_SSD1289
	#include "tft_drivers/ssd1289/setxy.h"
#endif
#ifndef DISABLE_ILI9325C
	#include "tft_drivers/ili9325c/setxy.h"
#endif
#ifndef DISABLE_ILI9325D
	#include "tft_drivers/ili9325d/default/setxy.h"
#endif
#ifndef DISABLE_ILI9325D_ALT
	#include "tft_drivers/ili9325d/alt/setxy.h"
#endif
#ifndef DISABLE_HX8340B_8
	#include "tft_drivers/hx8340b/8/setxy.h"
#endif
#ifndef DISABLE_HX8340B_S
	#include "tft_drivers/hx8340b/s/setxy.h"
#endif
#ifndef DISABLE_ST7735
	#include "tft_drivers/st7735/std/setxy.h"
#endif
#ifndef DISABLE_S1D19122
	#include "tft_drivers/s1d19122/setxy.h"
#endif
#ifndef DISABLE_PCF8833
	#include "tft_drivers/pcf8833/setxy.h"
#endif
#ifndef DISABLE_SSD1963_480
	#include "tft_drivers/ssd1963/480/setxy.h"
#endif
#ifndef DISABLE_SSD1963_800
	#include "tft_drivers/ssd1963/800/setxy.h"
#endif
#ifndef DISABLE_SSD1963_800_ALT
	#include "tft_drivers/ssd1963/800alt/setxy.h"
#endif
#ifndef DISABLE_S6D1121
	#include "tft_drivers/s6d1121/setxy.h"
#endif
#ifndef DISABLE_ILI9481
	#include "tft_drivers/ili9481/setxy.h"
#endif
#ifndef DISABLE_S6D0164
	#include "tft_drivers/s6d0164/setxy.h"
#endif
#ifndef DISABLE_ILI9341_S4P
	#include "tft_drivers/ili9341/s4p/setxy.h"
#endif
#ifndef DISABLE_ILI9341_S5P
	#include "tft_drivers/ili9341/s5p/setxy.h"
#endif
#ifndef DISABLE_ILI9341_8
	#include "tft_drivers/ili9341/8/setxy.h"
#endif
#ifndef DISABLE_ILI9341_16
	#include "tft_drivers/ili9341/16/setxy.h"
#endif
#ifndef DISABLE_R61581
	#include "tft_drivers/r61581/setxy.h"
#endif
#ifndef DISABLE_ILI9486
	#include "tft_drivers/ili9486/setxy.h"
#endif
#ifndef DISABLE_CPLD
	#include "tft_drivers/cpld/setxy.h"
#endif
#ifndef DISABLE_HX8353C
	#include "tft_drivers/hx8353c/setxy.h"
#endif
#ifndef DISABLE_ST7735_8
	#include "tft_drivers/st7735/8/setxy.h"
#endif
	}
}

void UTFT_clrXY()
{
	if (UTFT_orient==PORTRAIT)
		UTFT_setXY(0,0,UTFT_disp_x_size,UTFT_disp_y_size);
	else
		UTFT_setXY(0,0,UTFT_disp_y_size,UTFT_disp_x_size);
}

void UTFT_drawRect(int x1, int y1, int x2, int y2)
{
	if (x1>x2)
	{
		swap(int, x1, x2);
	}
	if (y1>y2)
	{
		swap(int, y1, y2);
	}

	UTFT_drawHLine(x1, y1, x2-x1);
	UTFT_drawHLine(x1, y2, x2-x1);
	UTFT_drawVLine(x1, y1, y2-y1);
	UTFT_drawVLine(x2, y1, y2-y1);
}

void UTFT_drawRoundRect(int x1, int y1, int x2, int y2)
{
	if (x1>x2)
	{
		swap(int, x1, x2);
	}
	if (y1>y2)
	{
		swap(int, y1, y2);
	}
	if ((x2-x1)>4 && (y2-y1)>4)
	{
		UTFT_drawPixel(x1+1,y1+1);
		UTFT_drawPixel(x2-1,y1+1);
		UTFT_drawPixel(x1+1,y2-1);
		UTFT_drawPixel(x2-1,y2-1);
		UTFT_drawHLine(x1+2, y1, x2-x1-4);
		UTFT_drawHLine(x1+2, y2, x2-x1-4);
		UTFT_drawVLine(x1, y1+2, y2-y1-4);
		UTFT_drawVLine(x2, y1+2, y2-y1-4);
	}
}

void UTFT_fillRect(int x1, int y1, int x2, int y2)
{
	if (x1>x2)
	{
		swap(int, x1, x2);
	}
	if (y1>y2)
	{
		swap(int, y1, y2);
	}
	if (UTFT_display_transfer_mode==16)
	{
		cbi(UTFT_P_CS, UTFT_B_CS);
		UTFT_setXY(x1, y1, x2, y2);
		sbi(UTFT_P_RS, UTFT_B_RS);
		UTFT__fast_fill_16(UTFT_fch,UTFT_fcl,(( ((long) x2-x1)+1)*( ((long)y2-y1)+1)));
		sbi(UTFT_P_CS, UTFT_B_CS);
	}
	else if ((UTFT_display_transfer_mode==8) && (UTFT_fch==UTFT_fcl))
	{
		cbi(UTFT_P_CS, UTFT_B_CS);
		UTFT_setXY(x1, y1, x2, y2);
		sbi(UTFT_P_RS, UTFT_B_RS);
		UTFT__fast_fill_8(UTFT_fch,(((long) x2-x1)+1)*(((long)y2-y1)+1));
		sbi(UTFT_P_CS, UTFT_B_CS);
	}
	else
	{
		if (UTFT_orient==PORTRAIT)
		{
			for (int i=0; i<((y2-y1)/2)+1; i++)
			{
				UTFT_drawHLine(x1, y1+i, x2-x1);
				UTFT_drawHLine(x1, y2-i, x2-x1);
			}
		}
		else
		{
			for (int i=0; i<((x2-x1)/2)+1; i++)
			{
				UTFT_drawVLine(x1+i, y1, y2-y1);
				UTFT_drawVLine(x2-i, y1, y2-y1);
			}
		}
	}
}

void UTFT_fillRoundRect(int x1, int y1, int x2, int y2)
{
	if (x1>x2)
	{
		swap(int, x1, x2);
	}
	if (y1>y2)
	{
		swap(int, y1, y2);
	}

	if ((x2-x1)>4 && (y2-y1)>4)
	{
		for (int i=0; i<((y2-y1)/2)+1; i++)
		{
			switch(i)
			{
			case 0:
				UTFT_drawHLine(x1+2, y1+i, x2-x1-4);
				UTFT_drawHLine(x1+2, y2-i, x2-x1-4);
				break;
			case 1:
				UTFT_drawHLine(x1+1, y1+i, x2-x1-2);
				UTFT_drawHLine(x1+1, y2-i, x2-x1-2);
				break;
			default:
				UTFT_drawHLine(x1, y1+i, x2-x1);
				UTFT_drawHLine(x1, y2-i, x2-x1);
			}
		}
	}
}

void UTFT_drawCircle(int x, int y, int radius)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x1 = 0;
	int y1 = radius;

  // chip select active
  GPIO_ResetBits(LCD_CHIP_SELECT__PORT, LCD_CHIP_SELECT__PIN);

	UTFT_setXY(x, y + radius, x, y + radius);
	UTFT_LCD_Write_DATA(UTFT_fch,UTFT_fcl);
	UTFT_setXY(x, y - radius, x, y - radius);
	UTFT_LCD_Write_DATA(UTFT_fch,UTFT_fcl);
	UTFT_setXY(x + radius, y, x + radius, y);
	UTFT_LCD_Write_DATA(UTFT_fch,UTFT_fcl);
	UTFT_setXY(x - radius, y, x - radius, y);
	UTFT_LCD_Write_DATA(UTFT_fch,UTFT_fcl);
 
	while(x1 < y1)
	{
		if(f >= 0) 
		{
			y1--;
			ddF_y += 2;
			f += ddF_y;
		}
		x1++;
		ddF_x += 2;
		f += ddF_x;    
		UTFT_setXY(x + x1, y + y1, x + x1, y + y1);
		UTFT_LCD_Write_DATA(UTFT_fch,UTFT_fcl);
		UTFT_setXY(x - x1, y + y1, x - x1, y + y1);
		UTFT_LCD_Write_DATA(UTFT_fch,UTFT_fcl);
		UTFT_setXY(x + x1, y - y1, x + x1, y - y1);
		UTFT_LCD_Write_DATA(UTFT_fch,UTFT_fcl);
		UTFT_setXY(x - x1, y - y1, x - x1, y - y1);
		UTFT_LCD_Write_DATA(UTFT_fch,UTFT_fcl);
		UTFT_setXY(x + y1, y + x1, x + y1, y + x1);
		UTFT_LCD_Write_DATA(UTFT_fch,UTFT_fcl);
		UTFT_setXY(x - y1, y + x1, x - y1, y + x1);
		UTFT_LCD_Write_DATA(UTFT_fch,UTFT_fcl);
		UTFT_setXY(x + y1, y - x1, x + y1, y - x1);
		UTFT_LCD_Write_DATA(UTFT_fch,UTFT_fcl);
		UTFT_setXY(x - y1, y - x1, x - y1, y - x1);
		UTFT_LCD_Write_DATA(UTFT_fch,UTFT_fcl);
	}

	// chip select no active
  GPIO_SetBits(LCD_CHIP_SELECT__PORT, LCD_CHIP_SELECT__PIN);

	UTFT_clrXY();
}

void UTFT_fillCircle(int x, int y, int radius)
{
	for(int y1=-radius; y1<=0; y1++) 
		for(int x1=-radius; x1<=0; x1++)
			if(x1*x1+y1*y1 <= radius*radius) 
			{
				UTFT_drawHLine(x+x1, y+y1, 2*(-x1));
				UTFT_drawHLine(x+x1, y-y1, 2*(-x1));
				break;
			}
}

void UTFT_clrScr()
{
	long i;
	
  // chip select active
  GPIO_ResetBits(LCD_CHIP_SELECT__PORT, LCD_CHIP_SELECT__PIN);

	UTFT_clrXY();
	if (UTFT_display_transfer_mode!=1)
  {
    // data mode
    GPIO_SetBits(LCD_COMMAND_DATA__PORT, LCD_COMMAND_DATA__PIN);
  }

	if (UTFT_display_transfer_mode==16)
		UTFT__fast_fill_16(0,0,((UTFT_disp_x_size+1)*(UTFT_disp_y_size+1)));
	else if (UTFT_display_transfer_mode==8)
		UTFT__fast_fill_8(0,((UTFT_disp_x_size+1)*(UTFT_disp_y_size+1)));
	else
	{
		for (i=0; i<((UTFT_disp_x_size+1)*(UTFT_disp_y_size+1)); i++)
		{
			if (UTFT_display_transfer_mode!=1)
				UTFT_LCD_Write_Bus(0,0,UTFT_display_transfer_mode);
			else
			{
				UTFT_LCD_Write_Bus(1,0,UTFT_display_transfer_mode);
				UTFT_LCD_Write_Bus(1,0,UTFT_display_transfer_mode);
			}
		}
	}

  // chip select no active
  GPIO_SetBits(LCD_CHIP_SELECT__PORT, LCD_CHIP_SELECT__PIN);
}

void UTFT_fillScr_rgb(byte r, byte g, byte b)
{
	word color = ((r&248)<<8 | (g&252)<<3 | (b&248)>>3);
	UTFT_fillScr(color);
}

void UTFT_fillScr(word color)
{
	long i;
	char ch, cl;
	
	ch=(byte) color>>8;
	cl=(byte) color & 0xFF;

  // chip select active
  GPIO_ResetBits(LCD_CHIP_SELECT__PORT, LCD_CHIP_SELECT__PIN);

	UTFT_clrXY();
	if (UTFT_display_transfer_mode!=1)
  {
    // data mode
    GPIO_SetBits(LCD_COMMAND_DATA__PORT, LCD_COMMAND_DATA__PIN);
  }

	if (UTFT_display_transfer_mode==16)
		UTFT__fast_fill_16(ch,cl,((UTFT_disp_x_size+1)*(UTFT_disp_y_size+1)));
	else if ((UTFT_display_transfer_mode==8) && (ch==cl))
		UTFT__fast_fill_8(ch,((UTFT_disp_x_size+1)*(UTFT_disp_y_size+1)));
	else
	{
		for (i=0; i<((UTFT_disp_x_size+1)*(UTFT_disp_y_size+1)); i++)
		{
			if (UTFT_display_transfer_mode!=1)
				UTFT_LCD_Write_Bus(ch,cl,UTFT_display_transfer_mode);
			else
			{
				UTFT_LCD_Write_Bus(1,ch,UTFT_display_transfer_mode);
				UTFT_LCD_Write_Bus(1,cl,UTFT_display_transfer_mode);
			}
		}
	}

  // chip select no active
  GPIO_SetBits(LCD_CHIP_SELECT__PORT, LCD_CHIP_SELECT__PIN);
}

void UTFT_setColor_rgb(byte r, byte g, byte b)
{
	UTFT_fch=((r&248)|g>>5);
	UTFT_fcl=((g&28)<<3|b>>3);
}

void UTFT_setColor(word color)
{
	UTFT_fch=(byte) (color>>8);
	UTFT_fcl=(byte) (color & 0xFF);
}

word UTFT_getColor()
{
	return (UTFT_fch<<8) | UTFT_fcl;
}

void UTFT_setBackColor_rgb(byte r, byte g, byte b)
{
	UTFT_bch=((r&248)|g>>5);
	UTFT_bcl=((g&28)<<3|b>>3);
	UTFT__transparent=false;
}

void UTFT_setBackColor(uint32_t color)
{
	if (color==VGA_TRANSPARENT)
		UTFT__transparent=true;
	else
	{
		UTFT_bch=(byte) (color>>8);
		UTFT_bcl=(byte) (color & 0xFF);
		UTFT__transparent=false;
	}
}

word UTFT_getBackColor()
{
	return (UTFT_bch<<8) | UTFT_bcl;
}

void UTFT_setPixel(word color)
{
	UTFT_LCD_Write_DATA((color>>8),(color&0xFF));	// rrrrrggggggbbbbb
}

void UTFT_drawPixel(int x, int y)
{
	cbi(UTFT_P_CS, UTFT_B_CS);
	UTFT_setXY(x, y, x, y);
	UTFT_setPixel((UTFT_fch<<8)|UTFT_fcl);
	sbi(UTFT_P_CS, UTFT_B_CS);
	UTFT_clrXY();
}

void UTFT_drawLine(int x1, int y1, int x2, int y2)
{
	if (y1==y2)
		UTFT_drawHLine(x1, y1, x2-x1);
	else if (x1==x2)
		UTFT_drawVLine(x1, y1, y2-y1);
	else
	{
		unsigned int	dx = (x2 > x1 ? x2 - x1 : x1 - x2);
		short			xstep =  x2 > x1 ? 1 : -1;
		unsigned int	dy = (y2 > y1 ? y2 - y1 : y1 - y2);
		short			ystep =  y2 > y1 ? 1 : -1;
		int				col = x1, row = y1;

		cbi(UTFT_P_CS, UTFT_B_CS);
		if (dx < dy)
		{
			int t = - (dy >> 1);
			while (true)
			{
				UTFT_setXY (col, row, col, row);
				UTFT_LCD_Write_DATA (UTFT_fch, UTFT_fcl);
				if (row == y2)
					return;
				row += ystep;
				t += dx;
				if (t >= 0)
				{
					col += xstep;
					t   -= dy;
				}
			} 
		}
		else
		{
			int t = - (dx >> 1);
			while (true)
			{
				UTFT_setXY (col, row, col, row);
				UTFT_LCD_Write_DATA (UTFT_fch, UTFT_fcl);
				if (col == x2)
					return;
				col += xstep;
				t += dy;
				if (t >= 0)
				{
					row += ystep;
					t   -= dx;
				}
			} 
		}
		sbi(UTFT_P_CS, UTFT_B_CS);
	}
	UTFT_clrXY();
}

void UTFT_drawHLine(int x, int y, int l)
{
	if (l<0)
	{
		l = -l;
		x -= l;
	}
	cbi(UTFT_P_CS, UTFT_B_CS);
	UTFT_setXY(x, y, x+l, y);
	if (UTFT_display_transfer_mode == 16)
	{
		sbi(UTFT_P_RS, UTFT_B_RS);
		UTFT__fast_fill_16(UTFT_fch,UTFT_fcl,l);
	}
	else if ((UTFT_display_transfer_mode==8) && (UTFT_fch==UTFT_fcl))
	{
		sbi(UTFT_P_RS, UTFT_B_RS);
		UTFT__fast_fill_8(UTFT_fch,l);
	}
	else
	{
		for (int i=0; i<l+1; i++)
		{
			UTFT_LCD_Write_DATA(UTFT_fch, UTFT_fcl);
		}
	}
	sbi(UTFT_P_CS, UTFT_B_CS);
	UTFT_clrXY();
}

void UTFT_drawVLine(int x, int y, int l)
{
	if (l<0)
	{
		l = -l;
		y -= l;
	}
	cbi(UTFT_P_CS, UTFT_B_CS);
	UTFT_setXY(x, y, x, y+l);
	if (UTFT_display_transfer_mode == 16)
	{
		sbi(UTFT_P_RS, UTFT_B_RS);
		UTFT__fast_fill_16(UTFT_fch,UTFT_fcl,l);
	}
	else if ((UTFT_display_transfer_mode==8) && (UTFT_fch==UTFT_fcl))
	{
		sbi(UTFT_P_RS, UTFT_B_RS);
		UTFT__fast_fill_8(UTFT_fch,l);
	}
	else
	{
		for (int i=0; i<l+1; i++)
		{
			UTFT_LCD_Write_DATA(UTFT_fch, UTFT_fcl);
		}
	}
	sbi(UTFT_P_CS, UTFT_B_CS);
	UTFT_clrXY();
}

void UTFT_printChar(byte c, int x, int y)
{
	byte i,ch;
	word j;
	word temp; 

	cbi(UTFT_P_CS, UTFT_B_CS);
  
	if (!UTFT__transparent)
	{
		if (UTFT_orient==PORTRAIT)
		{
			UTFT_setXY(x,y,x+UTFT_cfont.x_size-1,y+UTFT_cfont.y_size-1);
	  
			temp=((c-UTFT_cfont.offset)*((UTFT_cfont.x_size/8)*UTFT_cfont.y_size))+4;
			for(j=0;j<((UTFT_cfont.x_size/8)*UTFT_cfont.y_size);j++)
			{
				ch=pgm_read_byte(&UTFT_cfont.font[temp]);
				for(i=0;i<8;i++)
				{   
					if((ch&(1<<(7-i)))!=0)   
					{
						UTFT_setPixel((UTFT_fch<<8)|UTFT_fcl);
					} 
					else
					{
						UTFT_setPixel((UTFT_bch<<8)|UTFT_bcl);
					}   
				}
				temp++;
			}
		}
		else
		{
			temp=((c-UTFT_cfont.offset)*((UTFT_cfont.x_size/8)*UTFT_cfont.y_size))+4;

			for(j=0;j<((UTFT_cfont.x_size/8)*UTFT_cfont.y_size);j+=(UTFT_cfont.x_size/8))
			{
				UTFT_setXY(x,y+(j/(UTFT_cfont.x_size/8)),x+UTFT_cfont.x_size-1,y+(j/(UTFT_cfont.x_size/8)));
				for (int zz=(UTFT_cfont.x_size/8)-1; zz>=0; zz--)
				{
					ch=pgm_read_byte(&UTFT_cfont.font[temp+zz]);
					for(i=0;i<8;i++)
					{   
						if((ch&(1<<i))!=0)   
						{
							UTFT_setPixel((UTFT_fch<<8)|UTFT_fcl);
						} 
						else
						{
							UTFT_setPixel((UTFT_bch<<8)|UTFT_bcl);
						}   
					}
				}
				temp+=(UTFT_cfont.x_size/8);
			}
		}
	}
	else
	{
		temp=((c-UTFT_cfont.offset)*((UTFT_cfont.x_size/8)*UTFT_cfont.y_size))+4;
		for(j=0;j<UTFT_cfont.y_size;j++) 
		{
			for (int zz=0; zz<(UTFT_cfont.x_size/8); zz++)
			{
				ch=pgm_read_byte(&UTFT_cfont.font[temp+zz]); 
				for(i=0;i<8;i++)
				{   
				
					if((ch&(1<<(7-i)))!=0)   
					{
						UTFT_setXY(x+i+(zz*8),y+j,x+i+(zz*8)+1,y+j+1);
						UTFT_setPixel((UTFT_fch<<8)|UTFT_fcl);
					} 
				}
			}
			temp+=(UTFT_cfont.x_size/8);
		}
	}

	sbi(UTFT_P_CS, UTFT_B_CS);
	UTFT_clrXY();
}

void UTFT_rotateChar(byte c, int x, int y, int pos, int deg)
{
	byte i,j,ch;
	word temp; 
	int newx,newy;
	double radian;
	radian=deg*0.0175;  

	cbi(UTFT_P_CS, UTFT_B_CS);

	temp=((c-UTFT_cfont.offset)*((UTFT_cfont.x_size/8)*UTFT_cfont.y_size))+4;
	for(j=0;j<UTFT_cfont.y_size;j++) 
	{
		for (int zz=0; zz<(UTFT_cfont.x_size/8); zz++)
		{
			ch=pgm_read_byte(&UTFT_cfont.font[temp+zz]); 
			for(i=0;i<8;i++)
			{   
				newx=x+(((i+(zz*8)+(pos*UTFT_cfont.x_size))*cos(radian))-((j)*sin(radian)));
				newy=y+(((j)*cos(radian))+((i+(zz*8)+(pos*UTFT_cfont.x_size))*sin(radian)));

				UTFT_setXY(newx,newy,newx+1,newy+1);
				
				if((ch&(1<<(7-i)))!=0)   
				{
					UTFT_setPixel((UTFT_fch<<8)|UTFT_fcl);
				} 
				else  
				{
					if (!UTFT__transparent)
						UTFT_setPixel((UTFT_bch<<8)|UTFT_bcl);
				}   
			}
		}
		temp+=(UTFT_cfont.x_size/8);
	}
	sbi(UTFT_P_CS, UTFT_B_CS);
	UTFT_clrXY();
}

void UTFT_print(char *st, int x, int y)
{
  UTFT_print_rotate(st, x, y, 0);
}

void UTFT_print_rotate(char *st, int x, int y, int deg)
{
	int stl, i;

	stl = strlen(st);

	if (UTFT_orient==PORTRAIT)
	{
	if (x==RIGHT)
		x=(UTFT_disp_x_size+1)-(stl*UTFT_cfont.x_size);
	if (x==CENTER)
		x=((UTFT_disp_x_size+1)-(stl*UTFT_cfont.x_size))/2;
	}
	else
	{
	if (x==RIGHT)
		x=(UTFT_disp_y_size+1)-(stl*UTFT_cfont.x_size);
	if (x==CENTER)
		x=((UTFT_disp_y_size+1)-(stl*UTFT_cfont.x_size))/2;
	}

	for (i=0; i<stl; i++)
		if (deg==0)
			UTFT_printChar(*st++, x + (i*(UTFT_cfont.x_size)), y);
		else
			UTFT_rotateChar(*st++, x, y, i, deg);
}

void UTFT_printNumI(long num, int x, int y)
{
  UTFT_printNumI_adv(num, x, y, 0, ' ');
}

void UTFT_printNumI_adv(long num, int x, int y, int length, char filler)
{
	char buf[25];
	char st[27];
	boolean neg=false;
	int c=0, f=0;
  
	if (num==0)
	{
		if (length!=0)
		{
			for (c=0; c<(length-1); c++)
				st[c]=filler;
			st[c]=48;
			st[c+1]=0;
		}
		else
		{
			st[0]=48;
			st[1]=0;
		}
	}
	else
	{
		if (num<0)
		{
			neg=true;
			num=-num;
		}
	  
		while (num>0)
		{
			buf[c]=48+(num % 10);
			c++;
			num=(num-(num % 10))/10;
		}
		buf[c]=0;
	  
		if (neg)
		{
			st[0]=45;
		}
	  
		if (length>(c+neg))
		{
			for (int i=0; i<(length-c-neg); i++)
			{
				st[i+neg]=filler;
				f++;
			}
		}

		for (int i=0; i<c; i++)
		{
			st[i+neg+f]=buf[c-i-1];
		}
		st[c+neg+f]=0;

	}

	UTFT_print(st,x,y);
}

void UTFT_printNumF(double num, byte dec, int x, int y)
{
  UTFT_printNumF_adv(num, dec, x, y, '.', 0 ,' ');
}

void UTFT_printNumF_adv(double num, byte dec, int x, int y, char divider, int length, char filler)
{
	char st[27];
	boolean neg=false;

	if (dec<1)
		dec=1;
	else if (dec>5)
		dec=5;

	if (num<0)
		neg = true;

	UTFT__convert_float(st, num, length, dec);

	if (divider != '.')
	{
		for (int i=0; i<sizeof(st); i++)
			if (st[i]=='.')
				st[i]=divider;
	}

	if (filler != ' ')
	{
		if (neg)
		{
			st[0]='-';
			for (int i=1; i<sizeof(st); i++)
				if ((st[i]==' ') || (st[i]=='-'))
					st[i]=filler;
		}
		else
		{
			for (int i=0; i<sizeof(st); i++)
				if (st[i]==' ')
					st[i]=filler;
		}
	}

	UTFT_print(st,x,y);
}

void UTFT_setFont(uint8_t* font)
{
	UTFT_cfont.font=font;
	UTFT_cfont.x_size=fontbyte(0);
	UTFT_cfont.y_size=fontbyte(1);
	UTFT_cfont.offset=fontbyte(2);
	UTFT_cfont.numchars=fontbyte(3);
}

uint8_t* UTFT_getFont()
{
	return UTFT_cfont.font;
}

uint8_t UTFT_getFontXsize()
{
	return UTFT_cfont.x_size;
}

uint8_t UTFT_getFontYsize()
{
	return UTFT_cfont.y_size;
}

void UTFT_drawBitmap(int x, int y, int sx, int sy, bitmapdatatype data)
{
  UTFT_drawBitmap_scale(x, y, sx, sy, data, 1);
}

void UTFT_drawBitmap_scale(int x, int y, int sx, int sy, bitmapdatatype data, int scale)
{
	unsigned int col;
	int tx, ty, tc, tsx, tsy;

	if (scale==1)
	{
		if (UTFT_orient==PORTRAIT)
		{
			cbi(UTFT_P_CS, UTFT_B_CS);
			UTFT_setXY(x, y, x+sx-1, y+sy-1);
			for (tc=0; tc<(sx*sy); tc++)
			{
				col=pgm_read_word(&data[tc]);
				UTFT_LCD_Write_DATA(col>>8,col & 0xff);
			}
			sbi(UTFT_P_CS, UTFT_B_CS);
		}
		else
		{
			cbi(UTFT_P_CS, UTFT_B_CS);
			for (ty=0; ty<sy; ty++)
			{
				UTFT_setXY(x, y+ty, x+sx-1, y+ty);
				for (tx=sx-1; tx>=0; tx--)
				{
					col=pgm_read_word(&data[(ty*sx)+tx]);
					UTFT_LCD_Write_DATA(col>>8,col & 0xff);
				}
			}
			sbi(UTFT_P_CS, UTFT_B_CS);
		}
	}
	else
	{
		if (UTFT_orient==PORTRAIT)
		{
			cbi(UTFT_P_CS, UTFT_B_CS);
			for (ty=0; ty<sy; ty++)
			{
				UTFT_setXY(x, y+(ty*scale), x+((sx*scale)-1), y+(ty*scale)+scale);
				for (tsy=0; tsy<scale; tsy++)
					for (tx=0; tx<sx; tx++)
					{
						col=pgm_read_word(&data[(ty*sx)+tx]);
						for (tsx=0; tsx<scale; tsx++)
							UTFT_LCD_Write_DATA(col>>8,col & 0xff);
					}
			}
			sbi(UTFT_P_CS, UTFT_B_CS);
		}
		else
		{
			cbi(UTFT_P_CS, UTFT_B_CS);
			for (ty=0; ty<sy; ty++)
			{
				for (tsy=0; tsy<scale; tsy++)
				{
					UTFT_setXY(x, y+(ty*scale)+tsy, x+((sx*scale)-1), y+(ty*scale)+tsy);
					for (tx=sx-1; tx>=0; tx--)
					{
						col=pgm_read_word(&data[(ty*sx)+tx]);
						for (tsx=0; tsx<scale; tsx++)
							UTFT_LCD_Write_DATA(col>>8,col & 0xff);
					}
				}
			}
			sbi(UTFT_P_CS, UTFT_B_CS);
		}
	}
	UTFT_clrXY();
}

void UTFT_drawBitmap_rotate(int x, int y, int sx, int sy, bitmapdatatype data, int deg, int rox, int roy)
{
	unsigned int col;
	int tx, ty, newx, newy;
	double radian;
	radian=deg*0.0175;  

	if (deg==0)
		UTFT_drawBitmap(x, y, sx, sy, data);
	else
	{
		cbi(UTFT_P_CS, UTFT_B_CS);
		for (ty=0; ty<sy; ty++)
			for (tx=0; tx<sx; tx++)
			{
				col=pgm_read_word(&data[(ty*sx)+tx]);

				newx=x+rox+(((tx-rox)*cos(radian))-((ty-roy)*sin(radian)));
				newy=y+roy+(((ty-roy)*cos(radian))+((tx-rox)*sin(radian)));

				UTFT_setXY(newx, newy, newx, newy);
				UTFT_LCD_Write_DATA(col>>8,col & 0xff);
			}
		sbi(UTFT_P_CS, UTFT_B_CS);
	}
	UTFT_clrXY();
}

void UTFT_lcdOff()
{
	cbi(UTFT_P_CS, UTFT_B_CS);
	switch (UTFT_display_model)
	{
	case PCF8833:
		UTFT_LCD_Write_COM(0x28);
		break;
	case CPLD:
		UTFT_LCD_Write_COM_DATA(0x01,0x0000);
		UTFT_LCD_Write_COM(0x0F);   
		break;
	}
	sbi(UTFT_P_CS, UTFT_B_CS);
}

void UTFT_lcdOn()
{
	cbi(UTFT_P_CS, UTFT_B_CS);
	switch (UTFT_display_model)
	{
	case PCF8833:
		UTFT_LCD_Write_COM(0x29);
		break;
	case CPLD:
		UTFT_LCD_Write_COM_DATA(0x01,0x0010);
		UTFT_LCD_Write_COM(0x0F);   
		break;
	}
	sbi(UTFT_P_CS, UTFT_B_CS);
}

void UTFT_setContrast(char c)
{
	cbi(UTFT_P_CS, UTFT_B_CS);
	switch (UTFT_display_model)
	{
	case PCF8833:
		if (c>64) c=64;
		UTFT_LCD_Write_COM(0x25);
		UTFT_LCD_Write_DATA_VL(c);
		break;
	}
	sbi(UTFT_P_CS, UTFT_B_CS);
}

int UTFT_getDisplayXSize()
{
	if (UTFT_orient==PORTRAIT)
		return UTFT_disp_x_size+1;
	else
		return UTFT_disp_y_size+1;
}

int UTFT_getDisplayYSize()
{
	if (UTFT_orient==PORTRAIT)
		return UTFT_disp_y_size+1;
	else
		return UTFT_disp_x_size+1;
}

void UTFT_setBrightness(byte br)
{
	cbi(UTFT_P_CS, UTFT_B_CS);
	switch (UTFT_display_model)
	{
	case CPLD:
		if (br>16) br=16;
		UTFT_LCD_Write_COM_DATA(0x01,br);
		UTFT_LCD_Write_COM(0x0F);   
		break;
	}
	sbi(UTFT_P_CS, UTFT_B_CS);
}

void UTFT_setDisplayPage(byte page)
{
	cbi(UTFT_P_CS, UTFT_B_CS);
	switch (UTFT_display_model)
	{
	case CPLD:
		if (page>7) page=7;
		UTFT_LCD_Write_COM_DATA(0x04,page);
		UTFT_LCD_Write_COM(0x0F);   
		break;
	}
	sbi(UTFT_P_CS, UTFT_B_CS);
}

void UTFT_setWritePage(byte page)
{
	cbi(UTFT_P_CS, UTFT_B_CS);
	switch (UTFT_display_model)
	{
	case CPLD:
		if (page>7) page=7;
		UTFT_LCD_Write_COM_DATA(0x05,page);
		UTFT_LCD_Write_COM(0x0F);   
		break;
	}
	sbi(UTFT_P_CS, UTFT_B_CS);
}
