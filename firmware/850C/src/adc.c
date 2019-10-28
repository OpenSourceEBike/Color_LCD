#include "adc.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_adc.h"
#include "stdio.h"

#include "pins.h"

/**
 * Per @casainho
 *
 * PA4
 * ADC01_IN4, measure battery voltage. Battery voltage first goes over D5 and then R29 and R31 forms the voltage divider. R29 is connected in one side to GND.

 R31 = 200kohms; R29 = 10kohms. Vout = Vin * (R2 / (R2+R1)); Vout = Vin * 0,048.
 */
void adc_init() {
	GPIO_InitTypeDef ginit;
	ginit.GPIO_Pin = GPIO_Pin_4;
	ginit.GPIO_Mode = GPIO_Mode_AIN; //GPIO Pin as analog Mode
	ginit.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOA, &ginit); // GPIO Initialization

	ADC_DeInit(ADC1); //Deinitialize the ADC to reconfigure it
	ADC_InitTypeDef ADC_InitStruct;
	ADC_StructInit(&ADC_InitStruct);
	ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_Init(ADC1, &ADC_InitStruct);
	ADC_Cmd(ADC1, ENABLE);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 1, ADC_SampleTime_239Cycles5);

	// Queue up a conversion
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

#define NUM_STEPS 4096 // 12 bit

uint16_t battery_voltage_10x_get() {
  static uint8_t ui8_first_time = 1;
  uint32_t rawVoltage;

  // the very first measure need to be discarded as it seems to have a wrong value
  if (ui8_first_time) {
    ui8_first_time = 0;

    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
      ;

    rawVoltage = (uint32_t) ADC_GetConversionValue(ADC1);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE); // start a conversion for next time.
  }

	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
		;

	rawVoltage = (uint32_t) ADC_GetConversionValue(ADC1);
	uint32_t v_1000x = (3300UL * rawVoltage) / NUM_STEPS; // voltage at input pin x1000
	uint32_t busvolt_10x = (v_1000x * 2083) / 1000 / 10;

	ADC_SoftwareStartConvCmd(ADC1, ENABLE); // start a conversion for next time.
	return busvolt_10x;
}
