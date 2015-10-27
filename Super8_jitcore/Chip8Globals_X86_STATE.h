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
			SELF_MODIFYING_CODE = 4
		} X86_STATUS_CODE;

		extern uint8_t * x86_resume_address;
		extern uint16_t x86_resume_c8_pc;
		extern X86_STATUS_CODE x86_status_code;
	}
}