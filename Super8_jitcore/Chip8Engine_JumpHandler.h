#pragma once

#include <cstdint>
#include "Chip8Globals.h"

struct JUMP_ENTRY {
	uint16_t c8_address_to;
	uint8_t * x86_address_to; // JMP_M_PTR_32 uses this value
	uint8_t filled_flag;
};

struct COND_JUMP_ENTRY {
	uint16_t c8_address_from; // not used except for debug?
	uint16_t c8_address_to; // not used except for debug?
	uint8_t * x86_address_jump_value; // Contains absolute cache address pointing to the relative jump value (which will need to be filled in)
	uint8_t translator_cycles;
	uint8_t written_flag;
};

class Chip8Engine_JumpHandler
{
public:
	FastArrayList<JUMP_ENTRY> jump_list;
	FastArrayList<COND_JUMP_ENTRY> cond_jump_list;

	Chip8Engine_JumpHandler();
	~Chip8Engine_JumpHandler();

	int32_t recordJumpEntry(uint16_t c8_to_);
	int32_t findJumpEntry(uint16_t c8_to_);
	JUMP_ENTRY * getJumpEntryByIndex(uint32_t index);
	void invalidateJumpByIndex(int32_t index);
	void clearFilledFlagByC8PC(uint16_t c8_pc);

	int32_t recordConditionalJumpEntry(uint16_t c8_from_, uint16_t c8_to_, uint8_t translator_cycles_, uint8_t * x86_address_jump_value_);
	int32_t findConditionalJumpEntry(uint16_t c8_from_, uint16_t c8_to_);
	void decreaseConditionalCycle();
	uint8_t checkConditionalCycle();

	void checkAndFillJumpsByStartC8PC();
	void checkAndFillConditionalJumpsByCycles();

	void DEBUG_printJumpList();
	void DEBUG_printCondJumpList();
};

