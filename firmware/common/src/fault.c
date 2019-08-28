
#include "screen.h"
#include "ugui_config.h"
#include "fonts.h"
#include "stdlib.h"
#include "fault.h"
#include "state.h"

#ifdef SW102
#include "hardfault.h"
#include "nrf_nvic.h"
#include "nrf_delay.h"
#else

/**@brief Structure containing info about an error of the type @ref NRF_FAULT_ID_SDK_ERROR.
 */
typedef struct {
	uint16_t line_num; /**< The line number where the error occurred. */
	uint8_t const *p_file_name; /**< The file in which the error occurred. */
	uint32_t err_code; /**< The error code representing the error that occurred. */
} error_info_t;

#endif

/* Note: we currently don't use-funwind-tables because it adds about 8K to text.  But if we ever need better crash reports, turn them on and use the functions
 * in unwind.h to derive the PC of the failing function and a stack trace.

 #include <unwind.h>
 */

Field faultHeading = FIELD_DRAWTEXT(.msg = "FAULT");
Field faultCode = FIELD_DRAWTEXT();
Field addrHeading = FIELD_DRAWTEXT(.msg = "PC");
Field addrCode = FIELD_DRAWTEXT();
Field infoHeading = FIELD_DRAWTEXT(.msg = "Info");
Field infoCode = FIELD_DRAWTEXT();

Screen faultScreen = { .fields = { { .height = -1, .color = ColorInvert,
		.field = &faultHeading, .font = &TITLE_TEXT_FONT },

{ .y = -1, .height = -1, .color = ColorNormal, .field = &faultCode, .font =
		&REGULAR_TEXT_FONT }, { .y = -1, .height = -1, .color = ColorNormal,
		.field = &addrHeading, .font = &REGULAR_TEXT_FONT }, { .y = -1,
		.height = -1, .color = ColorNormal, .field = &addrCode, .font =
				&REGULAR_TEXT_FONT },
		{ .y = -1, .width = 0, .height = -1, .color = ColorNormal, .field =
				&infoHeading, .font = &REGULAR_TEXT_FONT }, { .y = -1, .height =
				-1, .color = ColorNormal, .field = &infoCode, .font =
				&REGULAR_TEXT_FONT }, { .field = NULL } } };

static inline void debugger_break(void) {
	__asm volatile(
			"bkpt #0x01\n\t"
			"mov pc, lr\n\t"
	);
}

/**@brief       Callback function for errors, asserts, and faults.
 *
 * @details     This function is called every time an error is raised in app_error, nrf_assert, or
 *              in the SoftDevice.
 *
 *              DEBUG_NRF flag must be set to trigger asserts!
 *
 */
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info) {
	fieldPrintf(&faultCode, "0x%lx", id);
	fieldPrintf(&addrCode, "0x%06lx", pc);

	switch (id) {
	case FAULT_GCC_ASSERT:
	case FAULT_SOFTDEVICE:
#ifdef SW102
  case NRF_FAULT_ID_SDK_ERROR:
#endif
	{
		// app errors include filename and line
		error_info_t *einfo = (error_info_t*) info;
		fieldPrintf(&infoCode, "%s:%d (%d)",
				einfo->p_file_name ?
						(const char*) einfo->p_file_name : "nofile",
				einfo->line_num, einfo->err_code);
		break;
	}

	case FAULT_HARDFAULT:
#ifdef SW102
    if(!info)
      fieldPrintf(&infoCode, "hf overflow");
    else {
      HardFault_stack_t *hs = (HardFault_stack_t *) info;

      fieldPrintf(&infoCode, "%x:%x",
        hs->r12, hs->lr);
    }
#endif
		break;

	case FAULT_MISSEDTICK:
		fieldPrintf(&infoCode, "missed tick");
		break;
	case FAULT_STACKOVERFLOW:
		fieldPrintf(&infoCode, "stack overflow");
		break;
	case FAULT_LOSTRX:
		fieldPrintf(&infoCode, "lost rx");
		break;
	default:
		fieldPrintf(&infoCode, "%08lx", info);
		break;
	}

	panicScreenShow(&faultScreen);

	if (is_sim_motor)
		debugger_break(); // if debugging, try to drop into the debugger

	// loop until the user presses the pwr button then reboot
	buttons_clear_all_events(); // require a new press
	while (1) {
		if (buttons_get_onoff_click_event())
#ifdef SW102
        nrf_delay_ms(20);
      sd_nvic_SystemReset();
#else
			; // FIXME
#endif

		buttons_clock(); // Note: this is done _after_ button events is checked to provide a 20ms debounce
	}
}

// handle standard gcc assert failures
void __attribute__((noreturn)) __assert_func(const char *file, int line,
		const char *func, const char *failedexpr) {
	error_info_t errinfo = { .line_num = (uint16_t) line, .p_file_name =
			(uint8_t const*) file, .err_code = FAULT_GCC_ASSERT };

	app_error_fault_handler(FAULT_GCC_ASSERT, 0, (uint32_t) &errinfo);
	abort();
}

