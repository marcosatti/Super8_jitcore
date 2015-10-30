#include "stdafx.h"
#include "Chip8Globals_X86_STATE.h"

namespace Chip8Globals {
	namespace X86_STATE {
		uint8_t * x86_resume_address;
		uint16_t x86_resume_c8_pc;
		uint16_t x86_resume_c8_return_pc;
		X86_STATUS_CODE x86_status_code;

		void DEBUG_printX86_STATE()
		{
			printf("X86_STATE: x86_resume_address = 0x%.8X, x86_resume_c8_pc = 0x%.4X, x86_status_code = %d\n", (uint32_t)x86_resume_address, x86_resume_c8_pc, x86_status_code);
		}
	}
}