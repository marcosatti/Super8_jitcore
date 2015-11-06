#include "stdafx.h"
#include "Chip8Globals_X86_STATE.h"

namespace Chip8Globals {
	namespace X86_STATE {
		uint8_t * x86_resume_address;
		uint16_t x86_interrupt_c8_param1;
		uint16_t x86_interrupt_c8_param2;
		uint8_t * x86_interrupt_x86_param1; // used with out of code interrupts
		X86_INT_STATUS_CODE x86_interrupt_status_code;

		void DEBUG_printX86_STATE()
		{
			printf("X86_STATE: x86_resume_address = 0x%.8X, x86_resume_c8_pc = 0x%.4X, x86_status_code = %d\n", (uint32_t)x86_resume_address, x86_interrupt_c8_param1, x86_interrupt_status_code);
		}
	}
}