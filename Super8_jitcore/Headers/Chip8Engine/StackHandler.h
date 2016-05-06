#pragma once

#include <cstdint>
#include <string>

#include "Headers\Globals.h"

#define NUM_STACK_LVLS 16

namespace Chip8Engine {

	struct STACK_ENTRY {
		uint16_t c8_address;
	};

	class StackHandler : ILogComponent
	{
	public:
		uint8_t * x86_address_to; // Pointer that is referenced by the x86 code in caches, whenever a stack jump is performed.

		StackHandler();
		~StackHandler();

		std::string getComponentName();

		// Stack handler, implements a simple stack. Only used in conjunction with the PREPARE_FOR_STACK_JUMP interrupt; see MainEngine.h/cpp.

		void resetStack();
		void setTopStack(STACK_ENTRY entry);
		STACK_ENTRY getTopStack();

#ifdef USE_DEBUG_EXTRA
		void DEBUG_printStack();
#endif

	private:
		STACK_ENTRY stack[NUM_STACK_LVLS]; // Stack used in the implementation, supports 16 levels.
		uint8_t sp; // Stack pointer
	};

}