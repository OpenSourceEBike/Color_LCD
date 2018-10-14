// *** Hardwarespecific functions ***
void UTFT__hw_special_init()
{
}

void UTFT_LCD_Write_Bus(char VH,char VL, byte mode)
{
	switch (mode)
	{
	  case 16:
	    GPIO_Write(GPIOB, ((((uint16_t) VH) << 8) + VL));
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

}

void UTFT__fast_fill_8(int ch, long pix)
{
//	long blocks;
//
//	//PORTD = ch;
//	UTFT__set_bits(ch);
//
//	blocks = pix/16;
//	for (int i=0; i<blocks; i++)
//	{
//		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
//		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
//		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
//		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
//		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
//		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
//		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
//		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
//		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
//		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
//		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
//		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
//		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
//		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
//		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
//		pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
//	}
//	if ((pix % 16) != 0)
//		for (int i=0; i<(pix % 16)+1; i++)
//		{
//			pulse_low(UTFT_P_WR, UTFT_B_WR);pulse_low(UTFT_P_WR, UTFT_B_WR);
//		}
}
