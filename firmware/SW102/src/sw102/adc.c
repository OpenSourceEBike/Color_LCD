#include "app_error.h"
#include "nrf_drv_adc.h"

static uint16_t adc_in_milli_volts;

#define ADC_REF_VOLTAGE_IN_MILLIVOLTS     600  // /< Reference voltage (in milli volts) used by ADC while doing conversion.
#define ADC_RES_10BIT                     1024 // /< Maximum digital value for 10-bit ADC conversion.
#define ADC_PRE_SCALING_COMPENSATION      6    // /< The ADC is configured to use VDD with 1/3 prescaling as input. And hence the result of conversion is to be multiplied by 3 to get the actual value of the battery voltage.
#define ADC_REF_VBG_VOLTAGE_IN_MILLIVOLTS 1200 // /< Value in millivolts for voltage used as reference in ADC conversion on NRF51.
#define ADC_INPUT_PRESCALER               3    // /< Input prescaler for ADC convestion on NRF51.

static nrf_adc_value_t adc_buf;
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE) \
    ((((ADC_VALUE) *ADC_REF_VBG_VOLTAGE_IN_MILLIVOLTS) / ADC_RES_10BIT) * ADC_INPUT_PRESCALER)

static void adc_event_handler(nrf_drv_adc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_ADC_EVT_DONE)
    {
        nrf_adc_value_t adc_result;

        adc_result = p_event->data.done.p_buffer[0];

        APP_ERROR_CHECK(nrf_drv_adc_buffer_convert(p_event->data.done.p_buffer, 1));

        adc_in_milli_volts =
            ADC_RESULT_IN_MILLI_VOLTS(adc_result);
    }
}


void battery_voltage_init(void)
{
    ret_code_t err_code = nrf_drv_adc_init(NULL, adc_event_handler);

    APP_ERROR_CHECK(err_code);

    static nrf_drv_adc_channel_t channel =
        NRF_DRV_ADC_DEFAULT_CHANNEL(NRF_ADC_CONFIG_INPUT_2);
    channel.config.config.input = (uint32_t)ADC_CONFIG_INPSEL_AnalogInputOneThirdPrescaling;
    nrf_drv_adc_channel_enable(&channel);

    APP_ERROR_CHECK(nrf_drv_adc_buffer_convert(&adc_buf, 1));
    nrf_drv_adc_sample();
}


uint16_t battery_voltage_10x_get()
{
  // @LowPerformer discovered there is a voltage divider 300k/16k on this input from the 19-48V battery
    uint16_t r =  adc_in_milli_volts / 10 * (300 + 16) / 16 / 10;

    // Queue a new conversion
    if (!nrf_adc_is_busy())
        nrf_drv_adc_sample();

    return r;
}
