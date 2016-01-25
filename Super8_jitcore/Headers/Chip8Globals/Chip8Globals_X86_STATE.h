#pragma once

#include <cstdint>

namespace Chip8Globals {
	namespace X86_STATE {
		// Status codes used in the x86 state
		enum X86_INT_STATUS_CODE {
			PREPARE_FOR_JUMP = 0,
			USE_INTERPRETER = 1,
			OUT_OF_CODE = 2,
			PREPARE_FOR_INDIRECT_JUMP = 3,
			SELF_MODIFYING_CODE = 4,
			DEBUG = 5,
			WAIT_FOR_KEYPRESS = 6,
			PREPARE_FOR_STACK_JUMP = 7,
			UPDATE_TIMERS = 8
		};

		extern uint8_t * x86_resume_address; // Used as the entry point into dynarec emulation.
		extern uint16_t x86_interrupt_c8_param1; // Used with many interrupts.
		extern uint16_t x86_interrupt_c8_param2; // Used with PREPARE_FOR_STACK_JUMP interrupts.
		extern uint8_t * x86_interrupt_x86_param1; // Used with out of code interrupts (to determine which cache needs more code).
		extern X86_INT_STATUS_CODE x86_interrupt_status_code; // Used by dispatcher loop to determine which type of interrupt happened.

#ifdef USE_DEBUG
		extern char * x86_int_status_code_strings[];
		extern void DEBUG_printX86_STATE();
#endif

	}
}