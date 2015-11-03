#include "stdafx.h"
#include "Chip8Engine.h"
#include <time.h>

Chip8Engine::Chip8Engine() {
}

Chip8Engine::~Chip8Engine() {
	delete key;
	delete stack;
	delete timers;
	delete interpreter;
	delete dynarec;
	delete cache;
	delete jumptbl;
}

void Chip8Engine::initialise() {
	// Initialise & reset timers
	timers = new Chip8TimersEngine();
	key = new Chip8KeyEngine();
	interpreter = new Chip8Engine_Interpreter();
	dynarec = new Chip8Engine_Dynarec();
	cache = new Chip8Engine_CacheHandler();
	stack = new Chip8Engine_StackHandler();
	jumptbl = new Chip8Engine_JumpHandler();

	translate_cycles = 0;

	C8_STATE::C8_allocMem();
	C8_STATE::cpu.pc = (uint16_t)0x200;					// Program counter starts at 0x200
	C8_STATE::opcode = (uint16_t)0x0000;				// Reset current opcode	
	C8_STATE::cpu.I = (uint16_t)0x000;					// Reset index register
	stack->resetStack();								// Reset stack pointer
#ifndef USE_SDL
	C8_STATE::C8_clearGFXMem();							// Clear display	
#endif
	C8_STATE::C8_clearRegV();							// Clear registers V0-VF
	C8_STATE::C8_clearMem();							// Clear memory

	// Load fontset
	memcpy(C8_STATE::memory, C8_STATE::chip8_fontset, FONTSET_SZ);

	// Setup/update cache here pop/push etc
	cache->setupCache_CDECL();

	// Setup first memory region
	cache->initFirstCache();
}

void Chip8Engine::loadProgram(std::string path) {
	// Load whole program into memory, starting at address 0x200.
	// Open file.
	std::ifstream file(path, std::ios::in | std::ios::binary);

	// Get length of file, store end location in global var
	file.seekg(0, std::ios::end);
	size_t length = (size_t) file.tellg();
	C8_STATE::rom_sz = 0x0200 + (uint16_t)length;
	file.seekg(0, std::ios::beg);

	// Read file into memory at address 0x200.
	file.read((char *)(C8_STATE::memory + 0x200), length);
	file.close();
}

void Chip8Engine::emulationLoop()
{
	// The heart and soul of this emulator
	// Exec cache and cleanup & handle return interrupt code (first run will produce OUT_OF_CODE)
	cache->execCache_CDECL();
	//printf("Chip8Engine: Ran cache ok. Interrupt code = %d, (optional) C8 handle opcode = 0x%.4X\n", X86_STATE::x86_status_code, X86_STATE::x86_resume_c8_pc); 
	// Flush caches that are marked
	cache->invalidateCacheByFlag();
	// Handle Interrupts
	handleInterrupt();
	//printf("Chip8Engine: NEW X86_RESUME_ADDRESS = 0x%.8X (in cache[%d])\n", (uint32_t)X86_STATE::x86_resume_address, cache->getCacheIndexByX86Address(X86_STATE::x86_resume_address));
}

void Chip8Engine::handleInterrupt()
{
	switch (X86_STATE::x86_status_code) {
	case X86_STATE::PREPARE_FOR_JUMP:
	{
		// ! ! ! X86_STATE::x86_resume_c8_pc contains jump location ! ! !

		// Need to update the jump table/cache before the jumps are made.
		//cache->DEBUG_printCacheList();
		jumptbl->checkAndFillJumpsByStartC8PC();
		break;
	}
	case X86_STATE::USE_INTERPRETER:
	{
		// ! ! ! X86_STATE::x86_resume_c8_pc contains interpreter opcode ! ! !

		// Opcode hasnt been implemented in the dynarec yet, need to use interpreter
		interpreter->setOpcode(X86_STATE::x86_resume_c8_pc);
		interpreter->emulateCycle();
		//cache->DEBUG_printCacheList();
		//DEBUG_render();
		break;
	}
	case X86_STATE::OUT_OF_CODE:
	{
		// ! ! ! X86_STATE::x86_resume_c8_pc contains start pc of cache ! ! !

		// End of cache was reached, because of 3 posibilities:
		// 1. The cache is genuinely out of code due to translator loop exiting on X many cycles
		// 2. The cache contains code (ie: conditional jump) that goes over the end jump
		// 3. The cache has previously been split up into two caches due to a jump into the middle of the cache.
		// Case 2 and 3 mean that no jump will be at the end of the cache! -> They are the same sort of problem
		// Can determine between 1 & 2-3 by looking for a cache with the next pc as the start pc

		// We can determine which case it is by looking at the do not write flag, where 1 and 3. will have false, and 2. will have true.
		// For determining between 1 and 3, we can check for an existing cache and emit jump code if there is
		CACHE_REGION * region = cache->getCacheInfoByC8PC(X86_STATE::x86_resume_c8_pc);
		X86_STATE::x86_resume_address = region->x86_mem_address + region->x86_pc;
		if (cache->findCacheIndexByStartC8PC(region->c8_end_recompile_pc + 2) != -1 || region->stop_write_flag != 0) {
			// Case 2 & 3 - Absolute end of region reached -> Record & Emit jump to next region
			// First record jump in jump table if DNE (so it will get updated on every translator loop)
			int32_t tblindex = jumptbl->findJumpEntry(region->c8_end_recompile_pc + 2);
			if (tblindex == -1) {
				tblindex = jumptbl->recordJumpEntry(region->c8_end_recompile_pc + 2);
			}

			// Emit the jump
			int32_t old_index = cache->findCacheIndexCurrent();
			cache->switchCacheByC8PC(X86_STATE::x86_resume_c8_pc);
			emitter->DYNAREC_EMIT_INTERRUPT(X86_STATE::PREPARE_FOR_JUMP, region->c8_end_recompile_pc + 2);
			emitter->JMP_M_PTR_32((uint32_t*)&jumptbl->jump_list[tblindex]->x86_address_to);
			cache->setStopWriteFlagCurrent();
			cache->switchCacheByIndex(old_index);
		} 
		else {
			// Case 1 - Absolute end of region NOT reached -> start recompiling code again
			// Start recompiling code at end of cache c8pc
			C8_STATE::cpu.pc = region->c8_end_recompile_pc;
			translatorLoop();
		}		
		break;
	}
	case X86_STATE::PREPARE_FOR_INDIRECT_JUMP:
	{
		printf("TODO: Implement PREPARE_FOR_INDIRECT_JUMP !\n");
		break;
	}
	case X86_STATE::SELF_MODIFYING_CODE:
	{
		// ! ! ! X86_STATE::x86_resume_c8_pc contains opcode from translator ! ! !
		// Only 2 opcodes in the C8 specs that do this. For SMC, need to invalidate cache that the memory writes to
		switch (X86_STATE::x86_resume_c8_pc & 0xF0FF) {
		case 0xF033:
		{
			// 0xFX33: Splits the decimal representation of Vx into 3 locations: hundreds stored in address I, tens in address I+1, and ones in I+2.
			//cache->DEBUG_printCacheList();
			uint16_t I = C8_STATE::cpu.I;
			cache->setInvalidFlagByC8PC(C8_STATE::cpu.I);
			cache->setInvalidFlagByC8PC(C8_STATE::cpu.I+1);
			cache->setInvalidFlagByC8PC(C8_STATE::cpu.I+2);
			break;
		}
		case 0xF055:
		{
			// 0xFX55: Copies all current values in registers V0 -> Vx to memory starting at address I.
			uint8_t vx = (X86_STATE::x86_resume_c8_pc & 0x0F00) >> 8; // Need to bit shift by 8 to get to a single base16 digit.
			for (uint8_t i = 0; i <= vx; i++) {
				cache->setInvalidFlagByC8PC(C8_STATE::cpu.I + i);
			}
			break;
		}
		}
		break;
	}
	case X86_STATE::DEBUG:
	{
		printf("!!! Debug Interrupt, Opcode = 0x%.4X !!!\n", X86_STATE::x86_resume_c8_pc);
		C8_STATE::DEBUG_printC8_STATE();
		X86_STATE::DEBUG_printX86_STATE();
		cache->DEBUG_printCacheList();
		jumptbl->DEBUG_printJumpList();
		jumptbl->DEBUG_printCondJumpList();
		break;
	}
	case X86_STATE::WAIT_FOR_KEYPRESS:
	{
		// ! ! ! X86_STATE::x86_resume_c8_pc contains the C8 opcode ! ! !
		// Only one opcode: 0xFX0A: A key press is awaited, then stored in Vx.
		// For now this will do, however it should be handled by the parent object to the C8Engine
		// Check if there has been a key press, and if so, store it in key->x86_key_pressed
		uint8_t keystate = 0;
		for (int i = 0; i < NUM_KEYS; i++) {
			keystate = key->getKeyState(i); // Get the keystate from the key object.
			if (keystate == 1) {
				key->X86_KEY_PRESSED = i; // Set Vx to the key pressed (0x0 -> 0xF). See dynarec
				break;
			}
		}
		break;
	}
	case X86_STATE::PREPARE_FOR_STACK_JUMP:
	{
		// ! ! ! X86_STATE::x86_resume_c8_pc contains either: 0x2NNN (call, address = NNN) or 0x00EE (ret) ! ! !
		switch (X86_STATE::x86_resume_c8_pc & 0xF000) {
		case 0x2000:
		{
			// get jump location & return location
			uint16_t jump_c8_pc = X86_STATE::x86_resume_c8_pc & 0x0FFF;
			uint16_t return_c8_pc = X86_STATE::x86_resume_c8_return_pc;

			// Record stack entry for the return point - which will be the next opcode!
			STACK_ENTRY entry;
			entry.c8_address = return_c8_pc;
			stack->setTopStack(entry);

			// First record jump in jump table if DNE (so it will get updated on every translator loop)
			int32_t tblindex = jumptbl->findJumpEntry(jump_c8_pc);
			if (tblindex == -1) {
				tblindex = jumptbl->recordJumpEntry(jump_c8_pc);
			}

			// Need to check/alloc jump location caches
			cache->getCacheWritableByStartC8PC(jump_c8_pc);
			jumptbl->checkAndFillJumpsByStartC8PC();

			// Set stack->x86_address_to equal to jumptable location
			stack->x86_address_to = jumptbl->jump_list[tblindex]->x86_address_to;
			break;
		}
		case 0x0000:
		{
			// Get stack entry & set jump location
			STACK_ENTRY entry = stack->getTopStack();

			// First record jump in jump table if DNE (so it will get updated on every translator loop)
			int32_t tblindex = jumptbl->findJumpEntry(entry.c8_address);
			if (tblindex == -1) {
				tblindex = jumptbl->recordJumpEntry(entry.c8_address);
			}

			// Need to check/alloc jump location caches
			cache->getCacheWritableByStartC8PC(entry.c8_address);
			jumptbl->checkAndFillJumpsByStartC8PC();

			// Set stack->x86_address_to equal to jumptable location
			stack->x86_address_to = jumptbl->jump_list[tblindex]->x86_address_to;
			break;
		}
		}
	}
	}
}

void Chip8Engine::translatorLoop()
{
	do {
		//printf("\nChip8Engine: Running translator cycle: %d\n", translate_cycles);

		// Check and fill in conditional jumps & decrease num of cycles
		jumptbl->decreaseConditionalCycle();
		jumptbl->checkAndFillConditionalJumpsByCycles();

		// Bounds checking (do not translate outside of rom location)
		if (C8_STATE::cpu.pc > C8_STATE::rom_sz) {
			printf("Chip8Engine: Warning: C8 PC was outside of rom location! Running cache again as there is no code to translate (reset pc to 0x0200).\n");
			C8_STATE::cpu.pc = 0x0200;
			translate_cycles++;
			break;
		}

		// Select the right cache to use
		int32_t cache_index = cache->getCacheWritableByC8PC(C8_STATE::cpu.pc);

		// Has code already been compiled? (only valid if start != end c8 pc)
		CACHE_REGION * memory_region = cache->getCacheInfoByIndex(cache_index);
		if (C8_STATE::cpu.pc <= memory_region->c8_end_recompile_pc && memory_region->c8_start_recompile_pc != memory_region->c8_end_recompile_pc) {
			// This is when we have entered a block that already has compiled code in it... need to switch to end of the region/change region and recompile from there.
			// Update cpu.pc to the end C8 PC of region
			// End C8 PC is defined to be already compiled, so need to plus 2 for next opcode. However, if next opcode is in a different region, we need to select the correct one. Place a continue here so we keep getting the REAL end memory region.
			// When we loop again, the check memory region function will run again and give the correct region.
			
			printf("Chip8Engine: Warning: C8 PC was not at end of block. Old C8 PC = 0x%.4X, New C8 PC = 0x%.4X. Re-running translator loop to check memory region again.\n", C8_STATE::cpu.pc, cache->getEndC8PCCurrent() + 2);
			C8_STATE::cpu.pc = cache->getEndC8PCCurrent() + 2;
			// Update cycle number
			translate_cycles++;
			continue;
		}

		// We are now in a valid memory region and at the end of the block (ready to recompile again)
		// Fetch Opcode
		C8_STATE::opcode = C8_STATE::memory[C8_STATE::cpu.pc] << 8 | C8_STATE::memory[C8_STATE::cpu.pc + 1]; // We have 8-bit memory, but an opcode is 16-bits long. Need to construct opcode from 2 successive memory locations.

		// Update Timers
		dynarec->emulateTranslatorTimers();
		// Translate
		dynarec->emulateTranslatorCycle();

		// Update cycle number
		translate_cycles++;

	} while (translate_cycles % 16 != 0 || jumptbl->checkConditionalCycle() > 0); // Limit a cache update to 16 c8 opcodes at a time, but do not exit if there is a conditional cycle waiting to be updated
}

void Chip8Engine::DEBUG_renderGFXText()
{
	using namespace C8_STATE;

	printf("--- START RENDER ---\n\n");
	// Draw
	for (int y = 0; y < 32; ++y)
	{
		for (int x = 0; x < 64; ++x)
		{
			if (gfxmem[(y * 64) + x] == 0)
				printf("O");
			else
				printf(" ");
		}
		printf("\n");
	}
	printf("\n");
	printf("--- END RENDER ---\n\n");
}