#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace Chip8Globals {
	namespace X86_STATE {
		// Status codes used in the x86 state
		typedef enum {
			PREPARE_FOR_JUMP = 0,
			USE_INTERPRETER = 1,
			OUT_OF_CODE = 2,
			PREPARE_FOR_INDIRECT_JUMP = 3,
			SELF_MODIFYING_CODE = 4,
			DEBUG = 5,
			WAIT_FOR_KEYPRESS = 6,
			PREPARE_FOR_STACK_JUMP = 7
		} X86_INT_STATUS_CODE;

		extern uint8_t * x86_resume_address;
		extern uint16_t x86_interrupt_c8_param1;
		extern uint16_t x86_interrupt_c8_param2; // used with stack interrupts
		extern uint8_t * x86_interrupt_x86_param1; // used with out of code interrupts (to determine which cache needs more code)
		extern X86_INT_STATUS_CODE x86_interrupt_status_code;

		extern void DEBUG_printX86_STATE();
	}
}