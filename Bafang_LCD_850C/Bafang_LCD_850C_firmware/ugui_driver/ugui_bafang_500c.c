/*******************************************************************************
* File Name: ugui_SSD1963.c
*
* Description:
*  This is a driver for the uGui graphical library developed by 
*  Achim Döbler.
*  It is for SSD1963 graphic controller found in a lot of low cost graphics 
*  chinese displays an to be used with PSoC microcontrollers from Cypress.
*  Will test it with other PSoC micros as soon as I can.
*
* Note:
*  For more information about uGui...
*  Website: http://www.embeddedlightning.com/ugui/
*  Git: https://github.com/achimdoebler/UGUI
*  Forum: http://www.embeddedlightning.com/forum/
*  PDF Reference manual (excellent): http://www.embeddedlightning.com/download/reference-guide/?wpdmdl=205
*
*  Thanks to Achim Döbler for such a god job.
*
* Log version:
*  1.0 - June, 2018.       First version.
*
********************************************************************************
* Copyright (c) 2018 Andres F. Navas
* This driver follows the same license than the uGui library.
*******************************************************************************/

#include "../ugui/ugui.h"
#include "../ugui_driver/ugui_bafang_500c.h"
#include "../UTFT-STM8S/UTFT.h"
#include "../pins.h"
#include "../delay.h"

#define HDP (DISPLAY_WIDTH - 1)
#define VDP (DISPLAY_HEIGHT - 1)

UG_GUI ugui_lcd;

inline void Display_Reset()
{
//    Display_WriteCommand(0x01);         //Software reset
//    CyDelay(10);
}

void lcd_backlight (uint32_t ui32_state)
{
  if (ui32_state)
  {
    GPIO_SetBits(LCD_BACKLIGHT__PORT, LCD_BACKLIGHT__PIN);
  }
  else
  {
    GPIO_ResetBits(LCD_BACKLIGHT__PORT, LCD_BACKLIGHT__PIN);
  }
}

void Display_Init()
{
  // next step is needed to have PB3 and PB4 working as GPIO
  /* Disable the Serial Wire Jtag Debug Port SWJ-DP */
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = LCD_BACKLIGHT__PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LCD_BACKLIGHT__PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_READ__PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LCD_READ__PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_RESET__PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LCD_RESET__PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_COMMAND_DATA__PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LCD_COMMAND_DATA__PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_CHIP_SELECT__PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LCD_CHIP_SELECT__PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_WRITE__PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LCD_WRITE__PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = 0xffff;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  // enable backlight
  lcd_backlight (ENABLE);

  // keep RESET and READ pins always at 1
  GPIO_SetBits(LCD_RESET__PORT, LCD_RESET__PIN);
  GPIO_SetBits(LCD_READ__PORT, LCD_READ__PIN);

  // chip select active
  GPIO_ResetBits(LCD_CHIP_SELECT__PORT, LCD_CHIP_SELECT__PIN);

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

  delay_ms(120); // 120ms delay after leaving sleep

  UTFT_LCD_Write_COM(0x29); // set_display_on

  UTFT_LCD_Write_COM(0x36); // set_address_mode
  // Vertical Flip: Normal display
  // Horizontal Flip: Flipped display
  // RGB/BGR Order: Pixels sent in BGR order
  // Column Address Order: Right to Left
  // Page Address Order: Top to Bottom
  UTFT_LCD_Write_DATA_VL(0x0A);

  // chip select no active
  GPIO_SetBits(LCD_CHIP_SELECT__PORT, LCD_CHIP_SELECT__PIN);

  // Initialize global structure and set PSET to this.PSET.
  UG_Init(&ugui_lcd, Display_PixelSet, DISPLAY_WIDTH, DISPLAY_HEIGHT);

  // Register acceleratos.
  UG_DriverRegister(DRIVER_FILL_FRAME, (void*)HW_FillFrame);
  UG_DriverRegister(DRIVER_DRAW_LINE, (void*)HW_DrawLine);
//  UG_DriverRegister(DRIVER_DRAW_IMAGE, (void*)HW_DrawImage);
}

void Display_WindowSet(unsigned int s_x,unsigned int e_x,unsigned int s_y,unsigned int e_y)
{
  uint16_t x1, x2, y1, y2;

  x1 = s_x;
  x2 = e_x;
  y1 = s_y;
  y2 = e_y;

  // chip select active
  GPIO_ResetBits(LCD_CHIP_SELECT__PORT, LCD_CHIP_SELECT__PIN);

  // set XY
  UTFT_LCD_Write_COM(0x2a);
  UTFT_LCD_Write_DATA_VL(x1>>8);
  UTFT_LCD_Write_DATA_VL(x1);
  UTFT_LCD_Write_DATA_VL(x2>>8);
  UTFT_LCD_Write_DATA_VL(x2);
  UTFT_LCD_Write_COM(0x2b);
  UTFT_LCD_Write_DATA_VL(y1>>8);
  UTFT_LCD_Write_DATA_VL(y1);
  UTFT_LCD_Write_DATA_VL(y2>>8);
  UTFT_LCD_Write_DATA_VL(y2);
  UTFT_LCD_Write_COM(0x2c);

  // chip select no active
  GPIO_SetBits(LCD_CHIP_SELECT__PORT, LCD_CHIP_SELECT__PIN);
}

void Display_PixelSet(UG_S16 x, UG_S16 y, UG_COLOR c)
{
  if((x < 0) ||(x >= DISPLAY_WIDTH) || (y < 0) || (y >= DISPLAY_HEIGHT)) return;

  // chip select active
  GPIO_ResetBits(LCD_CHIP_SELECT__PORT, LCD_CHIP_SELECT__PIN);

  UTFT_setXY(x, y, x, y);

  // data
  GPIO_SetBits(LCD_COMMAND_DATA__PORT, LCD_COMMAND_DATA__PIN);

  GPIO_Write(GPIOB, c);
  LCD_WRITE__PORT->BRR = LCD_WRITE__PIN;
  LCD_WRITE__PORT->BSRR = LCD_WRITE__PIN;

  // chip select no active
  GPIO_SetBits(LCD_CHIP_SELECT__PORT, LCD_CHIP_SELECT__PIN);
}

UG_RESULT HW_FillFrame(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR c)
{
    uint16_t loopx, loopy;
    uint8_t data[3];
    
    data[0] = (c >> 16);
    data[1] = (c >> 8);
    data[2] = (c);
    
    if((x1 < 0) ||(x1 >= DISPLAY_WIDTH) || (y1 < 0) || (y1 >= DISPLAY_HEIGHT)) return UG_RESULT_FAIL;
    if((x2 < 0) ||(x2 >= DISPLAY_WIDTH) || (y2 < 0) || (y2 >= DISPLAY_HEIGHT)) return UG_RESULT_FAIL;

    // chip select active
    GPIO_ResetBits(LCD_CHIP_SELECT__PORT, LCD_CHIP_SELECT__PIN);

    // set XY
    UTFT_LCD_Write_COM(0x2a);
    UTFT_LCD_Write_DATA_VL(x1>>8);
    UTFT_LCD_Write_DATA_VL(x1);
    UTFT_LCD_Write_DATA_VL(x2>>8);
    UTFT_LCD_Write_DATA_VL(x2);
    UTFT_LCD_Write_COM(0x2b);
    UTFT_LCD_Write_DATA_VL(y1>>8);
    UTFT_LCD_Write_DATA_VL(y1);
    UTFT_LCD_Write_DATA_VL(y2>>8);
    UTFT_LCD_Write_DATA_VL(y2);
    UTFT_LCD_Write_COM(0x2c);

    for (loopx = x1; loopx < x2 + 1; loopx++)
    {
        for (loopy = y1; loopy < y2 + 1; loopy++)
        {
          GPIO_Write(GPIOB, c);
          LCD_WRITE__PORT->BRR = LCD_WRITE__PIN;
          LCD_WRITE__PORT->BSRR = LCD_WRITE__PIN;
        }
    }
    
    // chip select no active
    GPIO_SetBits(LCD_CHIP_SELECT__PORT, LCD_CHIP_SELECT__PIN);

    return UG_RESULT_OK;
}

UG_RESULT HW_DrawLine( UG_S16 x1 , UG_S16 y1 , UG_S16 x2 , UG_S16 y2 , UG_COLOR c )
{
    if((x1 < 0) ||(x1 >= DISPLAY_WIDTH) || (y1 < 0) || (y1 >= DISPLAY_HEIGHT)) return UG_RESULT_FAIL;
    if((x2 < 0) ||(x2 >= DISPLAY_WIDTH) || (y2 < 0) || (y2 >= DISPLAY_HEIGHT)) return UG_RESULT_FAIL;
    
    // If it is a vertical or a horizontal line, draw it.
    // If not, then use original drawline routine.
    if ((x1 == x2) || (y1 == y2)) 
    {
        HW_FillFrame(x1, y1, x2, y2, c);
        return UG_RESULT_OK;
    }
    
    return UG_RESULT_FAIL;
}

UG_RESULT HW_DrawImage(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, uint8_t *image, uint16_t pSize)
{
//
//    if((x1 < 0) ||(x1 >= DISPLAY_WIDTH) || (y1 < 0) || (y1 >= DISPLAY_HEIGHT)) return UG_RESULT_FAIL;
//    if((x2 < 0) ||(x2 >= DISPLAY_WIDTH) || (y2 < 0) || (y2 >= DISPLAY_HEIGHT)) return UG_RESULT_FAIL;
//
//    Display_WindowSet(x1,x2,y1,y2);
//
//    Display_WriteCommand(0x2c);
//    Display_WriteMultiData(image, pSize*3);
//
//    return UG_RESULT_OK;
}

/* [] END OF FILE */
