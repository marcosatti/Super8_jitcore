#include "stdafx.h"
#include "Chip8Globals_X86_STATE.h"

namespace Chip8Globals {
	namespace X86_STATE {
		uint8_t * x86_resume_address;
		uint16_t x86_resume_c8_pc;
		X86_STATUS_CODE x86_status_code;
	}
}