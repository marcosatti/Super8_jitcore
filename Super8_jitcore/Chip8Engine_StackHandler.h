#pragma once

#include <cstdint>
#define NUM_STACK_LVLS 16

struct STACK_ENTRY {
	uint16_t c8_address;
};

class Chip8Engine_StackHandler
{
public:
	STACK_ENTRY stack[NUM_STACK_LVLS]; // Stack used in the implementation, supports 16 levels.
	uint8_t sp; // Stack pointer

	uint8_t * x86_address_to;

	Chip8Engine_StackHandler();
	~Chip8Engine_StackHandler();

	void setStackLevel(uint8_t level);
	void resetStack();
	void setTopStack(STACK_ENTRY entry);
	STACK_ENTRY getTopStack();
private:
};
