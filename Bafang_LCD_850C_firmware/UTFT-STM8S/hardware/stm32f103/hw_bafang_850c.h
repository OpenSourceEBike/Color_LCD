#include "delay.h"

#include "pins.h"

//#define INVERT

// *** Hardwarespecific functions ***
void UTFT__hw_special_init()
{
}

void UTFT_LCD_Write_Bus(char VH,char VL, byte mode)
{
  uint16_t ui16_data;

  ui16_data = (((uint16_t) VH) << 8) + ((uint16_t) VL);
  GPIO_Write(GPIOB, ui16_data);

  // pulse low WR pin
  GPIO_ResetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
  GPIO_SetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
}

void UTFT_LCD_Write_Bus_8(char VL)
{

}

void UTFT__set_direction_registers(byte mode)
{

}

void UTFT__fast_fill_16(int ch, int cl, long pix)
{
  // write color to bus
  GPIO_Write(GPIOB, ((((uint16_t) ch) << 8) + cl));

  while (pix > 0)
  {
    pix--;

    LCD_WRITE__PORT->BRR = LCD_WRITE__PIN;
    LCD_WRITE__PORT->BSRR = LCD_WRITE__PIN;
  }
}


void UTFT__fast_fill_8(int ch, long pix)
{
	long blocks;

//	//PORTD = ch;
//	UTFT__set_bits(ch);

	blocks = pix/16;
	for (int i=0; i<blocks; i++)
	{
		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
	}
	if ((pix % 16) != 0)
		for (int i=0; i<(pix % 16)+1; i++)
		{
			pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
		}
}
