#pragma once

#include <cstdint>

#include "Headers\Globals.h"

namespace Chip8Engine {

	struct JUMP_ENTRY {
		uint16_t c8_address_to; // Which Chip8 PC location this jump references.
		uint8_t * x86_address_to; // The x86 address that corresponds to the Chip8 jump location.
		uint8_t filled_flag; // Used to signify if this Chip8 PC jump location has a valid corresponding cache x86 address to jump to. This flag will be cleared if the cache has been invalidated due to self-modifying code.
	};

	struct COND_JUMP_ENTRY {
		uint16_t c8_address_from; // Not used except for debugging messages.
		uint16_t c8_address_to; // Not used except for debugging messages.
		uint32_t * x86_address_jump_value; // Contains absolute cache address pointing to the relative jump value location (which will filled in).
		uint8_t translator_cycles; // Used to signify to the translator loop when this conditional jump should be filled in. For the Chip8 system, this is always 1 (all conditional jumps skip the next Chip8 instruction).
	};

	class JumpHandler : ILogComponent
	{
	public:
		std::vector<JUMP_ENTRY> * jump_list;
		std::vector<COND_JUMP_ENTRY> * cond_jump_list;
		uint8_t * x86_indirect_jump_address; // USED ONLY FOR INDIRECT JUMPS! (as address to jump to might change, but we have already emitted a jump.. This is the easiest way to update the jump location).

		JumpHandler();
		~JumpHandler();

		std::string getComponentName();

		// This is the jump table, which holds mappings from/to Chip8 memory addresses <-> x86 memory addresses. In general when a jump is encountered in the x86 code, it will use a JMP_M_PTR_32 instruction to jump to the next cache, which references the jump table.

		// Functions below invlove the jump table, mostly only used by the interrupt handlers (OUT_OF_CODE and PREPARE_FOR_JUMP)
		int32_t getJumpIndexByC8PC(uint16_t c8_to);
		JUMP_ENTRY * findJumpInfoByIndex(uint32_t index);
		void clearFilledFlagByC8PC(uint16_t c8_pc);
		void checkAndFillJumpsByStartC8PC();

		// Functions below involve the conditional jump table, which is only used by the translator loop.
		int32_t recordConditionalJumpEntry(uint16_t c8_from_, uint16_t c8_to_, uint8_t translator_cycles_, uint32_t * x86_address_jump_value_);
		void decreaseConditionalCycle();
		void checkAndFillConditionalJumpsByCycles();

#ifdef USE_DEBUG_EXTRA
		void DEBUG_printJumpList();
		void DEBUG_printCondJumpList();
#endif

	private:
		int32_t recordJumpEntry(uint16_t c8_to_);
		int32_t findJumpEntry(uint16_t c8_to_);
	};

}