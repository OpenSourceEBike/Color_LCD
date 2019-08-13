

#include "screen.h"
#include "hardfault.h"
#include "fonts.h"
#include "stdlib.h"
#include "fault.h"
#include "main.h"
#include "nrf_nvic.h"
#include "nrf_delay.h"

Field faultHeading = FIELD_DRAWTEXT(&MY_FONT_8X12, .msg = "FAULT");
Field faultCode = FIELD_DRAWTEXT(&FONT_5X12);
Field addrHeading = FIELD_DRAWTEXT(&FONT_5X12, .msg = "PC");
Field addrCode = FIELD_DRAWTEXT(&FONT_5X12);
Field infoHeading = FIELD_DRAWTEXT(&FONT_5X12, .msg = "Info");
Field infoCode = FIELD_DRAWTEXT(&FONT_5X12);

Screen faultScreen = {
    .fields = {
    { .height = -1, .color = ColorInvert, .field = &faultHeading },
    { .y = FONT12_Y, .height = -1, .color = ColorNormal, .field = &faultCode },
    { .y = 2 * FONT12_Y, .height = -1, .color = ColorNormal,
    .field = &addrHeading },
    { .y = 3 * FONT12_Y, .height = -1, .color = ColorNormal, .field = &addrCode },
    { .y = 4 * FONT12_Y,
    .width = 0, .height = -1, .color = ColorNormal, .field = &infoHeading },
    { .y = 5 * FONT12_Y, .height = -1, .color = ColorNormal, .field = &infoCode },
    { .field = NULL }
    } };




static inline void debugger_break(void)
{
  __asm volatile(
      "bkpt #0x01\n\t"
      "mov pc, lr\n\t"
  );
}


// handle standard gcc assert failures
void __attribute__((noreturn)) __assert_func(const char *file, int line,
    const char *func, const char *failedexpr)
{
  error_info_t errinfo = { .line_num = (uint16_t) line, .p_file_name = (uint8_t const *) file, .err_code = FAULT_GCC_ASSERT };

  app_error_fault_handler(FAULT_GCC_ASSERT, 0, (uint32_t) &errinfo);
  abort();
}


/// Called for critical hardfaults by the CPU - note - p_stack might be null, if we've overrun our main stack, in that case stack history is unavailable
void HardFault_process  ( HardFault_stack_t *   p_stack ) {
  uint32_t pc = !p_stack ? 0 : p_stack->pc;

  app_error_fault_handler(FAULT_GCC_ASSERT, pc, (uint32_t) &p_stack);
}

/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name)
{
  error_info_t errinfo = { .line_num = line_num, .p_file_name = p_file_name, .err_code = FAULT_NRFASSERT };

  app_error_fault_handler(FAULT_NRFASSERT, 0, (uint32_t) &errinfo);
}

/**@brief       Callback function for errors, asserts, and faults.
 *
 * @details     This function is called every time an error is raised in app_error, nrf_assert, or
 *              in the SoftDevice.
 *
 *              DEBUG_NRF flag must be set to trigger asserts!
 *
 */
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
  /*
   UG_FontSelect(&MY_FONT_8X12);
   char buf[32];
   sprintf(buf, "ERR 0x%lx\n", id);
   UG_ConsolePutString(buf);

   UG_ConsolePutString("PC\n");
   UG_FontSelect(&FONT_5X12);
   sprintf(buf, "0x%lx\n", pc);
   UG_ConsolePutString(buf);
   UG_FontSelect(&MY_FONT_8X12);
   UG_ConsolePutString("INFO\n");
   UG_FontSelect(&FONT_5X12);
   sprintf(buf, "0x%lx\n", info);
   UG_ConsolePutString(buf);
   lcd_refresh();
   */

  fieldPrintf(&faultCode, "0x%lx", id);
  fieldPrintf(&addrCode, "0x%06lx", pc);

  error_info_t *einfo = (error_info_t*) info;

  switch (id)
  {
  case NRF_FAULT_ID_SDK_ERROR:
    // app errors include filename and line
    fieldPrintf(&infoCode, "%s:%d (%d)",
        einfo->p_file_name ? (const char*) einfo->p_file_name : "nofile",
        einfo->line_num, einfo->err_code);
    break;
  case FAULT_GCC_ASSERT:
    fieldPrintf(&infoCode, "%s:%d",
        einfo->p_file_name ? (const char*) einfo->p_file_name : "",
        einfo->line_num);
    break;
  case FAULT_HARDFAULT:
    if(!info)
      fieldPrintf(&infoCode, "stk overflow");
    else {
      HardFault_stack_t *hs = (HardFault_stack_t *) info;

      fieldPrintf(&infoCode, "%x:%x",
        hs->r12, hs->lr);
    }
    break;
  case FAULT_SOFTDEVICE:
    fieldPrintf(&infoCode, "softdevice");
    break;
  case FAULT_MISSEDTICK:
    fieldPrintf(&infoCode, "missed tick");
    break;
  case FAULT_STACKOVERFLOW:
    fieldPrintf(&infoCode, "stack overflow");
    break;
  default:
    fieldPrintf(&infoCode, "%08lx", info);
    break;
  }

  panicScreenShow(&faultScreen);

  //if(id == FAULT_SOFTDEVICE) (did not work - failed experiment)
  //  return; // kevinh, see if we can silently continue - softdevice might be messed up but at least we can continue debugging?

  if(is_sim_motor)
    debugger_break(); // if debugging, try to drop into the debugger

  // loop until the user presses the pwr button then reboot
  buttons_clear_all_events(); // require a new press
  while (1) {
    if(buttons_get_onoff_click_event())
      sd_nvic_SystemReset();

    nrf_delay_ms(20);
    buttons_clock(); // Note: this is done _after_ button events is checked to provide a 20ms debounce
  }
}
