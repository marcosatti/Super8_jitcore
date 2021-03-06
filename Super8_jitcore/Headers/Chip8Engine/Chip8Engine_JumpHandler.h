#pragma once

#include <cstdint>

#include "Headers\Globals.h"
#include "Headers\FastArrayList\FastArrayList.h"


struct JUMP_ENTRY {
	uint16_t c8_address_to;
	uint8_t * x86_address_to; // JMP_M_PTR_32 uses this value
};

struct COND_JUMP_ENTRY {
	uint16_t c8_address_from; // not used except for debug?
	uint16_t c8_address_to; // not used except for debug?
	uint32_t * x86_address_jump_value; // Contains absolute cache address pointing to the relative jump value (which will need to be filled in)
	uint8_t translator_cycles;
};

class Chip8Engine_JumpHandler : ILogComponent
{
public:
	FastArrayList<int32_t> * jump_fill_list;
	FastArrayList<JUMP_ENTRY> * jump_list;
	FastArrayList<COND_JUMP_ENTRY> * cond_jump_list;
	uint8_t * x86_indirect_jump_address; // USED ONLY FOR INDIRECT JUMPS! (as address to jump to might change, but we have already emitted a jump.. This is the easiest way to update the jump location).

	Chip8Engine_JumpHandler();
	~Chip8Engine_JumpHandler();

	std::string getComponentName();

	int32_t getJumpIndexByC8PC(uint16_t c8_to);
	JUMP_ENTRY * getJumpInfoByIndex(uint32_t index);
	void clearFilledFlagByC8PC(uint16_t c8_pc);

	int32_t recordConditionalJumpEntry(uint16_t c8_from_, uint16_t c8_to_, uint8_t translator_cycles_, uint32_t * x86_address_jump_value_);
	void decreaseConditionalCycle();
	uint8_t checkConditionalCycle();

	void checkAndFillJumpsByStartC8PC();
	void checkAndFillConditionalJumpsByCycles();

#ifdef USE_DEBUG_EXTRA
	void DEBUG_printJumpList();
	void DEBUG_printCondJumpList();
#endif

private:
	int32_t recordJumpEntry(uint16_t c8_to_);
	int32_t findJumpEntry(uint16_t c8_to_);
};
