#include "delay.h"

#include "pins.h"

//#define INVERT

// *** Hardwarespecific functions ***
void UTFT__hw_special_init()
{
}

void UTFT_LCD_Write_Bus(char VH,char VL, byte mode)
{
  delay(2);

  switch (mode)
  {
    case 16:
      GPIO_Write(GPIOB, ((((uint16_t) VH) << 8) + VL));
      delay(2);

      // pulse low WR pin
      GPIO_ResetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
      delay(2);
      GPIO_SetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
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

  GPIO_Write(GPIOB, ((((uint16_t) ch) << 8) + cl));

  blocks = pix/16;
  for (int i=0; i<blocks; i++)
  {
    GPIO_ResetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
    delay(2);
    GPIO_SetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);

    GPIO_ResetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
    delay(2);
    GPIO_SetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);

    GPIO_ResetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
    delay(2);
    GPIO_SetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);

    GPIO_ResetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
    delay(2);
    GPIO_SetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);

    GPIO_ResetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
    delay(2);
    GPIO_SetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);

    GPIO_ResetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
    delay(2);
    GPIO_SetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);

    GPIO_ResetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
    delay(2);
    GPIO_SetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);

    GPIO_ResetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
    delay(2);
    GPIO_SetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);

    GPIO_ResetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
    delay(2);
    GPIO_SetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);

    GPIO_ResetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
    delay(2);
    GPIO_SetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);

    GPIO_ResetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
    delay(2);
    GPIO_SetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);

    GPIO_ResetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
    delay(2);
    GPIO_SetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);

    GPIO_ResetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
    delay(2);
    GPIO_SetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);

    GPIO_ResetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
    delay(2);
    GPIO_SetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);

    GPIO_ResetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
    delay(2);
    GPIO_SetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);

    GPIO_ResetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
    delay(2);
    GPIO_SetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
  }
  if ((pix % 16) != 0)
    for (int i=0; i<(pix % 16)+1; i++)
    {
      GPIO_ResetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
      delay(2);
      GPIO_SetBits(LCD_WRITE__PORT, LCD_WRITE__PIN);
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
