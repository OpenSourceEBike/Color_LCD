

#include "screen.h"
#include "hardfault.h"
#include "fonts.h"
#include "stdlib.h"
#include "fault.h"
#include "state.h"
#include "nrf_nvic.h"
#include "nrf_delay.h"

/* Note: we currently don't use-funwind-tables because it adds about 8K to text.  But if we ever need better crash reports, turn them on and use the functions
 * in unwind.h to derive the PC of the failing function and a stack trace.

#include <unwind.h>
*/


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
