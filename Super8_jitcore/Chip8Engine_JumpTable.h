#pragma once
#include <cstdint>
#include <vector>

#include "Chip8Globals.h"

#define NUM_TRANSLATOR_STACK_LVLS 0xFFF // number of stack levels used (remember: a 'stack' is a LIFO system)

// WARNING: only supports 8-bit short jumps right now
// TODO: add support for near jumps. Can do it by reserving an extra byte with jump translations. If it only needs an 8-bit jump, can 0x90 NOP the extra byte.

// Also implements a basic stack for supporting call/return opcodes

struct JUMP_TABLE_ENTRY {
	uint8_t * x86_jump_from_address;
	uint8_t * x86_jump_to_address;
	uint16_t c8_jump_from_address;
	uint16_t c8_jump_to_address;
	uint8_t jump_num_bytes;
	bool jump_handled;
};

class Chip8Engine_JumpTable
{
public:
	std::vector<JUMP_TABLE_ENTRY> table;
	uint8_t * C8_stack[NUM_TRANSLATOR_STACK_LVLS]; // Stack used in the translator
	uint8_t C8_sp; // Stack pointer;

	Chip8Engine_JumpTable();
	~Chip8Engine_JumpTable();

	void handleRelativeJump(uint16_t c8_relative_offset, uint8_t jmp_num_bytes);
	void handleAbsoluteJump(uint16_t c8_absolute_address, uint8_t jmp_num_bytes);
	uint32_t checkAndFillJumps();
	bool isJumpsUnhandled();

	void resetC8Stack();
	void setC8TopStack(uint8_t * address);
	uint8_t * getC8TopStack();
private:
	
};

