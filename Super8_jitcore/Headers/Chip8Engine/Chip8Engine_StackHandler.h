#pragma once

#include <cstdint>
#include <string>

#include "Headers\Globals.h"

#define NUM_STACK_LVLS 16

struct STACK_ENTRY {
	uint16_t c8_address;
};

class Chip8Engine_StackHandler : ILogComponent
{
public:
	STACK_ENTRY stack[NUM_STACK_LVLS]; // Stack used in the implementation, supports 16 levels.
	uint8_t sp; // Stack pointer

	uint8_t * x86_address_to;

	Chip8Engine_StackHandler();
	~Chip8Engine_StackHandler();

	std::string getComponentName();

	void setStackLevel(uint8_t level);
	void resetStack();
	void setTopStack(STACK_ENTRY entry);
	STACK_ENTRY getTopStack();

#ifdef USE_DEBUG_EXTRA
	void DEBUG_printStack();
#endif

private:
};
