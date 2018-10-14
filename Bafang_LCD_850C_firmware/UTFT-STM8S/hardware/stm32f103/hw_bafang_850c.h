#include "delay.h"

//#define INVERT

// *** Hardwarespecific functions ***
void UTFT__hw_special_init()
{
}

void UTFT_LCD_Write_Bus(char VH,char VL, byte mode)
{
#if defined (INVERT)
  uint16_t ui16_temp;
  uint16_t ui16_temp_1;

  ui16_temp = (((uint16_t) VH) << 8) + VL;

  ui16_temp_1 = (((ui16_temp >> 15) & 1) << 0);
  ui16_temp_1 |= (((ui16_temp >> 14) & 1) << 1);
  ui16_temp_1 |= (((ui16_temp >> 13) & 1) << 2);
  ui16_temp_1 |= (((ui16_temp >> 12) & 1) << 3);
  ui16_temp_1 |= (((ui16_temp >> 11) & 1) << 4);
  ui16_temp_1 |= (((ui16_temp >> 10) & 1) << 5);
  ui16_temp_1 |= (((ui16_temp >> 9) & 1) << 6);
  ui16_temp_1 |= (((ui16_temp >> 8) & 1) << 7);
  ui16_temp_1 |= (((ui16_temp >> 7) & 1) << 8);
  ui16_temp_1 |= (((ui16_temp >> 6) & 1) << 9);
  ui16_temp_1 |= (((ui16_temp >> 5) & 1) << 10);
  ui16_temp_1 |= (((ui16_temp >> 4) & 1) << 11);
  ui16_temp_1 |= (((ui16_temp >> 3) & 1) << 12);
  ui16_temp_1 |= (((ui16_temp >> 2) & 1) << 13);
  ui16_temp_1 |= (((ui16_temp >> 1) & 1) << 14);
  ui16_temp_1 |= ((ui16_temp & 1) << 15);
#endif

  delay(2);

  switch (mode)
	{
	  case 16:
#if defined (INVERT)
	    GPIO_Write(GPIOB, ui16_temp_1);
#else
	    GPIO_Write(GPIOB, ((((uint16_t) VH) << 8) + VL));
#endif

	    delay(2);
	    pulse_low(UTFT_P_WR, UTFT_B_WR);
		break;
	}
}

void UTFT_LCD_Write_Bus_8(char VL)
{

}

void UTFT__set_direction_registers(byte mode)
{

}

void UTFT__fast_fill_16(int ch, int cl, long pix)
{
  long blocks;

#if defined (INVERT)
  uint16_t ui16_temp;
  uint16_t ui16_temp_1;

  ui16_temp = (((uint16_t) ch) << 8) + cl;

  ui16_temp_1 = (((ui16_temp >> 15) & 1) << 0);
  ui16_temp_1 |= (((ui16_temp >> 14) & 1) << 1);
  ui16_temp_1 |= (((ui16_temp >> 13) & 1) << 2);
  ui16_temp_1 |= (((ui16_temp >> 12) & 1) << 3);
  ui16_temp_1 |= (((ui16_temp >> 11) & 1) << 4);
  ui16_temp_1 |= (((ui16_temp >> 10) & 1) << 5);
  ui16_temp_1 |= (((ui16_temp >> 9) & 1) << 6);
  ui16_temp_1 |= (((ui16_temp >> 8) & 1) << 7);
  ui16_temp_1 |= (((ui16_temp >> 7) & 1) << 8);
  ui16_temp_1 |= (((ui16_temp >> 6) & 1) << 9);
  ui16_temp_1 |= (((ui16_temp >> 5) & 1) << 10);
  ui16_temp_1 |= (((ui16_temp >> 4) & 1) << 11);
  ui16_temp_1 |= (((ui16_temp >> 3) & 1) << 12);
  ui16_temp_1 |= (((ui16_temp >> 2) & 1) << 13);
  ui16_temp_1 |= (((ui16_temp >> 1) & 1) << 14);
  ui16_temp_1 |= ((ui16_temp & 1) << 15);

  GPIO_Write(GPIOB, ui16_temp_1);
#else
  GPIO_Write(GPIOB, ((((uint16_t) ch) << 8) + cl));
#endif

  blocks = pix/16;
  for (int i=0; i<blocks; i++)
  {
    pulse_low(UTFT_P_WR, UTFT_B_WR);
    pulse_low(UTFT_P_WR, UTFT_B_WR);
    pulse_low(UTFT_P_WR, UTFT_B_WR);
    pulse_low(UTFT_P_WR, UTFT_B_WR);
    pulse_low(UTFT_P_WR, UTFT_B_WR);
    pulse_low(UTFT_P_WR, UTFT_B_WR);
    pulse_low(UTFT_P_WR, UTFT_B_WR);
    pulse_low(UTFT_P_WR, UTFT_B_WR);
    pulse_low(UTFT_P_WR, UTFT_B_WR);
    pulse_low(UTFT_P_WR, UTFT_B_WR);
    pulse_low(UTFT_P_WR, UTFT_B_WR);
    pulse_low(UTFT_P_WR, UTFT_B_WR);
    pulse_low(UTFT_P_WR, UTFT_B_WR);
    pulse_low(UTFT_P_WR, UTFT_B_WR);
    pulse_low(UTFT_P_WR, UTFT_B_WR);
    pulse_low(UTFT_P_WR, UTFT_B_WR);
  }
  if ((pix % 16) != 0)
    for (int i=0; i<(pix % 16)+1; i++)
    {
      pulse_low(UTFT_P_WR, UTFT_B_WR);
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
