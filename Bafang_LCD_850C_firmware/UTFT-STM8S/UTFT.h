/*
  UTFT.h - Multi-Platform library support for Color TFT LCD Boards
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

#ifndef UTFT_h
#define UTFT_h

#define UTFT_VERSION	283

#define LEFT 0
#define RIGHT 9999
#define CENTER 9998

#define PORTRAIT 0
#define LANDSCAPE 1

#define HX8347A			0
#define ILI9327			1
#define SSD1289			2
#define ILI9325C		3
#define ILI9325D_8		4
#define ILI9325D_16		5
#define HX8340B_8		6
#define HX8340B_S		7
#define HX8352A			8
#define ST7735			9
#define PCF8833			10
#define S1D19122		11
#define SSD1963_480		12
#define SSD1963_800		13
#define S6D1121_8		14
#define S6D1121_16		15
#define	SSD1289LATCHED	16
#define ILI9341_16		17
#define ILI9341_8       18
#define SSD1289_8		19
#define	SSD1963_800ALT	20
#define ILI9481			21
#define ILI9325D_16ALT	22
#define S6D0164			23
#define NOT_IN_USE		24
#define ILI9341_S5P		25
#define ILI9341_S4P		26
#define R61581			27
#define ILI9486			28
#define CPLD			29
#define HX8353C			30
#define ST7735_8			31

#define ITDB32			0	// HX8347-A (16bit)
#define ITDB32WC		1	// ILI9327  (16bit)
#define ITDB32S			2	// SSD1289  (16bit)
#define CTE32			2	// SSD1289  (16bit)
#define ITDB24			3	// ILI9325C (8bit)
#define ITDB24D			4	// ILI9325D (8bit)
#define ITDB24DWOT		4	// ILI9325D (8bit)
#define ITDB28			4	// ILI9325D (8bit)
#define DMTFT24104      4   // ILI9325D (8bit)
#define DMTFT28103      4   // ILI9325D (8bit)
#define ITDB22			6	// HX8340-B (8bit)
#define ITDB22SP		7	// HX8340-B (Serial 4Pin)
#define ITDB32WD		8	// HX8352-A (16bit)
#define CTE32W			8	// HX8352-A	(16bit)
#define ITDB18SP		9	// ST7735   (Serial 5Pin)
#define LPH9135			10	// PCF8833	(Serial 5Pin)
#define ITDB25H			11	// S1D19122	(16bit)
#define ITDB43			12	// SSD1963	(16bit) 480x272
#define ITDB50			13	// SSD1963	(16bit) 800x480
#define CTE50			13	// SSD1963	(16bit) 800x480
#define EHOUSE50		13	// SSD1963	(16bit) 800x480
#define ITDB24E_8		14	// S6D1121	(8bit)
#define ITDB24E_16		15	// S6D1121	(16bit)
#define INFINIT32		16	// SSD1289	(Latched 16bit) -- Legacy, will be removed later
#define ELEE32_REVA		16	// SSD1289	(Latched 16bit)
#define CTE32_R2		17	// ILI9341	(16bit)
#define ITDB24E_V2_16   17  // ILI9341	(16bit)
#define ITDB28_V2_ALT   17  // ILI9341	(16bit)
#define ITDB32S_V2      17  // ILI9341	(16bit)
#define ITDB28_V2       18  // ILI9341	(8bit)
#define ITDB24E_V2_8    18  // ILI9341	(8bit) 
#define ELEE32_REVB		19	// SSD1289	(8bit)
#define CTE70			20	// SSD1963	(16bit) 800x480 Alternative Init
#define EHOUSE70		20	// SSD1963	(16bit) 800x480 Alternative Init
#define CTE32HR			21	// ILI9481	(16bit)
#define CTE28			22	// ILI9325D (16bit) Alternative Init
#define CTE22			23	// S6D0164	(8bit)
#define DMTFT22102      23  // S6D0164  (8bit)
#define DMTFT28105      25  // ILI9341  (Serial 5Pin)
#define MI0283QT9		26  // ILI9341	(Serial 4Pin)
#define CTE35IPS		27	// R61581	(16bit)
#define CTE40			28	// ILI9486	(16bit)
#define EHOUSE50CPLD	29	// CPLD		(16bit)
#define CTE50CPLD		29	// CPLD		(16bit)
#define CTE70CPLD		29	// CPLD		(16bit)
#define DMTFT18101      30  // HX8353C  (Serial 5Pin)

#define SERIAL_4PIN		4
#define SERIAL_5PIN		5
#define LATCHED_16		17

#define NOTINUSE		255

//*********************************
// COLORS
//*********************************
// VGA color palette
#define VGA_BLACK		0x0000
#define VGA_WHITE		0xFFFF
#define VGA_RED			0xF800
#define VGA_GREEN		0x0400
#define VGA_BLUE		0x001F
#define VGA_SILVER		0xC618
#define VGA_GRAY		0x8410
#define VGA_MAROON		0x8000
#define VGA_YELLOW		0xFFE0
#define VGA_OLIVE		0x8400
#define VGA_LIME		0x07E0
#define VGA_AQUA		0x07FF
#define VGA_TEAL		0x0410
#define VGA_NAVY		0x0010
#define VGA_FUCHSIA		0xF81F
#define VGA_PURPLE		0x8010
#define VGA_TRANSPARENT	0xFFFFFFFF


//#include "Arduino.h"
#include "stdio.h"
#include "hardware/stm8/HW_STM8_defines.h"
#include "hardware/stm32f103/hw_stm32f103_defines.h"

struct _current_font
{
	uint8_t* font;
	uint8_t x_size;
	uint8_t y_size;
	uint8_t offset;
	uint8_t numchars;
};

uint32_t UTFT_read_reg_0 (uint8_t ui8_reg);

		void	UTFT (void);
		void  UTFT_SER(byte model);
		void	UTFT_InitLCD();
		void	UTFT_InitLCD_orientation(byte orientation);
		void	UTFT_clrScr();
		void	UTFT_drawPixel(int x, int y);
		void	UTFT_drawLine(int x1, int y1, int x2, int y2);
		void	UTFT_fillScr_rgb(byte r, byte g, byte b);
		void	UTFT_fillScr(word color);
		void	UTFT_drawRect(int x1, int y1, int x2, int y2);
		void	UTFT_drawRoundRect(int x1, int y1, int x2, int y2);
		void	UTFT_fillRect(int x1, int y1, int x2, int y2);
		void	UTFT_fillRoundRect(int x1, int y1, int x2, int y2);
		void	UTFT_drawCircle(int x, int y, int radius);
		void	UTFT_fillCircle(int x, int y, int radius);
		void	UTFT_setColor_rgb(byte r, byte g, byte b);
		void	UTFT_setColor(word color);
		word	UTFT_getColor();
		void	UTFT_setBackColor_rgb(byte r, byte g, byte b);
		void	UTFT_setBackColor(uint32_t color);
		word	UTFT_getBackColor();
		void	UTFT_print(char *st, int x, int y); // int deg=0
		void	UTFT_print_rotate(char *st, int x, int y, int deg);
		void	UTFT_printNumI(long num, int x, int y); // int length=0, char filler=' '
		void	UTFT_printNumI_adv(long num, int x, int y, int length, char filler);
		void	UTFT_printNumF(double num, byte dec, int x, int y); // char divider='.', int length=0, char filler=' '
		void	UTFT_printNumF_adv(double num, byte dec, int x, int y, char divider, int length, char filler);
		void	UTFT_setFont(uint8_t* font);
		uint8_t* UTFT_getFont();
		uint8_t	UTFT_getFontXsize();
		uint8_t	UTFT_getFontYsize();
		void	UTFT_drawBitmap(int x, int y, int sx, int sy, bitmapdatatype data); // int scale=1
		void	UTFT_drawBitmap_scale(int x, int y, int sx, int sy, bitmapdatatype data, int scale);
		void	UTFT_drawBitmap_rotate(int x, int y, int sx, int sy, bitmapdatatype data, int deg, int rox, int roy);
		void	UTFT_lcdOff();
		void	UTFT_lcdOn();
		void	UTFT_setContrast(char c);
		int		UTFT_getDisplayXSize();
		int		UTFT_getDisplayYSize();
		void	UTFT_setBrightness(byte br);
		void	UTFT_setDisplayPage(byte page);
		void	UTFT_setWritePage(byte page);

/*
	The functions and variables below should not normally be used.
	They have been left publicly available for use in add-on libraries
	that might need access to the lower level functions of UTFT.

	Please note that these functions and variables are not documented
	and I do not provide support on how to use them.
*/
extern byte			UTFT_fch, UTFT_fcl, UTFT_bch, UTFT_bcl;
extern byte			UTFT_orient;
extern long			UTFT_disp_x_size, UTFT_disp_y_size;
extern byte			UTFT_display_model, UTFT_display_transfer_mode, UTFT_display_serial_mode;
extern regtype     *UTFT_P_SDA, *UTFT_P_SCL;
extern GPIO_TypeDef  *UTFT_P_RS, *UTFT_P_WR, *UTFT_P_CS, *UTFT_P_RST, *UTFT_P_ALE;
extern regsize      UTFT_B_SDA, UTFT_B_SCL;
extern uint16_t     UTFT_B_RS, UTFT_B_WR, UTFT_B_CS, UTFT_B_RST, UTFT_B_ALE;
extern struct _current_font	UTFT_cfont;
extern boolean			UTFT__transparent;

extern word			UTFT_offset_x, UTFT_offset_y;

		void UTFT_LCD_Write_Bus(char VH,char VL, byte mode);
		void UTFT_LCD_Write_COM(char VL);
		void UTFT_LCD_Write_DATA(char VH,char VL);
		void UTFT_LCD_Write_DATA_VL(char VL);
		void UTFT_LCD_Write_COM_DATA(char com1,int dat1);
		void UTFT_LCD_Write_Bus_8(char VL);
		void UTFT_LCD_Write_DATA_8(char VL);
		void UTFT__hw_special_init();
		void UTFT_setPixel(word color);
		void UTFT_drawHLine(int x, int y, int l);
		void UTFT_drawVLine(int x, int y, int l);
		void UTFT_printChar(byte c, int x, int y);
		void UTFT_setXY(word x1, word y1, word x2, word y2);
		void UTFT_clrXY();
		void UTFT_rotateChar(byte c, int x, int y, int pos, int deg);
		void UTFT__set_direction_registers(byte mode);
		void UTFT__fast_fill_16(int ch, int cl, long pix);
		void UTFT__fast_fill_8(int ch, long pix);
		void UTFT__convert_float(char *buf, double num, int width, byte prec);


#endif
