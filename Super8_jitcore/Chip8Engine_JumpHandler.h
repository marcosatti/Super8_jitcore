#pragma once

#include <cstdint>
#include <vector>
#include "Chip8Globals.h"

struct JUMP_ENTRY {
	uint16_t c8_address_from;
	uint16_t c8_address_to;
	uint8_t * x86_address_to; // JMP_M_PTR_32 uses this value
};

struct COND_JUMP_ENTRY {
	uint16_t c8_address_from; // not used except for debug?
	uint16_t c8_address_to; // not used except for debug?
	uint8_t translator_cycles;
	uint8_t written_flag;
	uint8_t * x86_address_jump_value; // Contains absolute cache address pointing to the relative jump value (which will need to be filled in)
};

class Chip8Engine_JumpHandler
{
public:
	std::vector<JUMP_ENTRY> jump_table;
	std::vector<COND_JUMP_ENTRY> cond_jump_table;

	Chip8Engine_JumpHandler();
	~Chip8Engine_JumpHandler();

	int32_t recordConditionalJumpEntry(uint16_t c8_from_, uint16_t c8_to_, uint8_t translator_cycles_, uint8_t * x86_address_jump_value_);
	int32_t findConditionalJumpEntry(uint16_t c8_from_, uint16_t c8_to_);
	void decreaseConditionalCycle();

	void invalidateJumpByIndex(int32_t index);
	JUMP_ENTRY * getJumpEntryByIndex(uint32_t index);
	int32_t recordJumpEntry(uint16_t c8_from_, uint16_t c8_to_);
	int32_t findJumpEntry(uint16_t c8_from_, uint16_t c8_to_);

	void checkAndFillJumpsByStartC8PC();
	void checkAndFillJumpsByCurrentC8PC();
	void checkAndFillConditionalJumpsByCycles();

	
};

