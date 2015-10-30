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
	dynarec_break_loop = false;

	C8_STATE::C8_allocMem();
	C8_STATE::cpu.pc = (uint16_t)0x200;					// Program counter starts at 0x200
	C8_STATE::opcode = (uint16_t)0x0000;				// Reset current opcode	
	C8_STATE::cpu.I = (uint16_t)0x000;					// Reset index register
	stack->resetStack();								// Reset stack pointer

	C8_STATE::C8_clearGFXMem();							// Clear display	
	C8_STATE::C8_clearRegV();							// Clear registers V0-VF
	C8_STATE::C8_clearMem();							// Clear memory

	// Load fontset
	memcpy(C8_STATE::memory, C8_STATE::chip8_fontset, FONTSET_SZ);

	// Setup/update cache here pop/push etc
	cache->setupCache_CDECL();

	// Setup first memory region
	dynarec->initialiseFirstMemoryRegion();
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
	while (true) {
		// Exec caches
		//cache->DEBUG_printCacheList();
		//jumptbl->DEBUG_printJumpList();
		//jumptbl->DEBUG_printCondJumpList();
		cache->execCache_CDECL();
		//printf("Chip8Engine: Ran cache ok. Interrupt code = %d, (optional) C8 handle opcode = 0x%.4X\n", X86_STATE::x86_status_code, X86_STATE::x86_resume_c8_pc); 
		// Flush caches that are marked
		cache->invalidateCacheByFlag();
		// Handle Interrupts
		handleX86Interrupt();
		//printf("Chip8Engine: NEW X86_RESUME_ADDRESS = 0x%.8X (in cache[%d])\n", (uint32_t)X86_STATE::x86_resume_address, cache->getCacheIndexByX86Address(X86_STATE::x86_resume_address));
	}
}

void Chip8Engine::handleX86Interrupt()
{
	// DEBUG: change key states everytime an interrupt is generated
	if (drawcycles % 32 == 0) {
		uint8_t randstate = 0;
		srand((unsigned int)time(NULL) + (unsigned int)drawcycles);
		for (int i = 0; i < 0x10; i++) {
			randstate = rand() % 0x2;
			setKeyState(i, (KEY_STATE)randstate);
		}
	}

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
		DEBUG_render();
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
		//cache->DEBUG_printCacheList();
		if (cache->getCacheIndexByStartC8PC(region->c8_end_recompile_pc + 2) != -1 || region->stop_write_flag > 0) {
			// Case 2 & 3 - Absolute end of region reached -> Record & Emit jump to next region
			// First record jump in jump table if DNE (so it will get updated on every translator loop)
			int32_t tblindex = jumptbl->findJumpEntry(region->c8_end_recompile_pc + 2);
			if (tblindex == -1) {
				tblindex = jumptbl->recordJumpEntry(region->c8_end_recompile_pc + 2);
			}

			// Emit the jump
			int32_t old_index = cache->getCacheIndexCurrent();
			cache->switchCacheByC8PC(X86_STATE::x86_resume_c8_pc);
			emitter->DYNAREC_EMIT_INTERRUPT(X86_STATE::PREPARE_FOR_JUMP, region->c8_end_recompile_pc + 2);
			emitter->JMP_M_PTR_32((uint32_t*)&jumptbl->jump_list[tblindex].x86_address_to);
			cache->setStopWriteFlagCurrent(1);
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
		uint8_t result = 0;
		switch (X86_STATE::x86_resume_c8_pc & 0xF0FF) {
		case 0xF033:
		{
			// 0xFX33: Splits the decimal representation of Vx into 3 locations: hundreds stored in address I, tens in address I+1, and ones in I+2.
			//cache->DEBUG_printCacheList();
			uint16_t I = C8_STATE::cpu.I;
			result = cache->setInvalidFlagByC8PC(C8_STATE::cpu.I);
			result = cache->setInvalidFlagByC8PC(C8_STATE::cpu.I+1);
			result = cache->setInvalidFlagByC8PC(C8_STATE::cpu.I+2);
			break;
		}
		case 0xF055:
		{
			// 0xFX55: Copies all current values in registers V0 -> Vx to memory starting at address I.
			uint8_t vx = (X86_STATE::x86_resume_c8_pc & 0x0F00) >> 8; // Need to bit shift by 8 to get to a single base16 digit.
			for (uint8_t i = 0; i <= vx; i++) {
				result = cache->setInvalidFlagByC8PC(C8_STATE::cpu.I + i);
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
		printf("Memory values at 0x02F2(+ 0,1,2) = 0x%.2X, 0x%.2X, 0x%.2X\n", C8_STATE::memory[0x02F2], C8_STATE::memory[0x02F3], C8_STATE::memory[0x02F4]);

		//X86_STATE::DEBUG_printX86_STATE();
		cache->DEBUG_printCacheList();
		//jumptbl->DEBUG_printJumpList();
		//jumptbl->DEBUG_printCondJumpList();
		break;
	}
	case X86_STATE::WAIT_FOR_KEYPRESS:
	{
		// ! ! ! X86_STATE::x86_resume_c8_pc contains NO USEFUL INFO ! ! !
		// For now this will do, however it should be handled by the parent object to the C8Engine
		// Check if there has been a key press, and if so, store it in key->x86_key_pressed
		uint8_t keystate = 0;
		for (int i = 0; i < NUM_KEYS; i++) {
			keystate = key->getKeyState(i); // Get the keystate from the key object.
			if (keystate) {
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
			cache->allocNewCacheByJumpC8PC(jump_c8_pc);
			jumptbl->checkAndFillJumpsByStartC8PC();

			// Set stack->x86_address_to equal to jumptable location
			stack->x86_address_to = jumptbl->jump_list[tblindex].x86_address_to;
			//printf("stack->x86_address_to = 0x%.8X\n", (uint32_t)stack->x86_address_to);
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
			cache->allocNewCacheByJumpC8PC(entry.c8_address);
			jumptbl->checkAndFillJumpsByStartC8PC();

			// Set stack->x86_address_to equal to jumptable location
			stack->x86_address_to = jumptbl->jump_list[tblindex].x86_address_to;
			//printf("stack->x86_address_to = 0x%.8X\n", (uint32_t)stack->x86_address_to);
			break;
		}
		}
	}
	}
}

int32_t Chip8Engine::translatorSelectCache()
{
	// First check if memory region is ready/allocated (check for both current pc (indicates already recompiled), and check for pc-2 (ready to write to))
	int32_t cache_index = cache->getCacheIndexByC8PC(C8_STATE::cpu.pc);
	if (cache_index != -1) {
		// Cache exists for current PC
		// Check if its invalid
		if (cache->getInvalidFlagByIndex(cache_index)) {
			// Cache was invalid, so create a new cache at current PC
			cache_index = cache->allocAndSwitchNewCacheByC8PC(C8_STATE::cpu.pc);
		}
		else {
			// Cache was not marked as invalid, so next check for the do not write flag.
			// However, it does not matter as later the pc will be moved to the end of the region (code must already exist in this logic block), so just switch to this cache for now
			cache->switchCacheByIndex(cache_index);
		}
	}
	else {
		// Cache does not exist for current PC, so it hasnt been compiled before. Need to get region which is available to hold this recompiled code
		// Check for cache with PC - 2 end PC.
		cache_index = cache->getCacheIndexByC8PC(C8_STATE::cpu.pc - 2);
		if (cache_index != -1) {
			// Cache exists for current PC - 2
			// Check if its invalid
			if (cache->getInvalidFlagByIndex(cache_index)) {
				// Cache was invalid, so create a new cache at current PC
				cache_index = cache->allocAndSwitchNewCacheByC8PC(C8_STATE::cpu.pc);
			}
			else {
				// Cache was not marked as invalid, so next check for the do not write flag.
				if (cache->getStopWriteFlagByIndex(cache_index)) {
					// Cache has do not write flag set, so allocate a new region
					cache_index = cache->allocAndSwitchNewCacheByC8PC(C8_STATE::cpu.pc);
				}
				else {
					// Cache does not have do not write flag set, so its safe to write to this cache
					cache->switchCacheByIndex(cache_index);
				}
			}
		}
		else {
			// No cache was found for PC - 2 as well, so allocate a new one for current pc
			cache_index = cache->allocAndSwitchNewCacheByC8PC(C8_STATE::cpu.pc);
		}
	}
	return cache_index;
}

void Chip8Engine::translatorLoop()
{
	uint8_t translator_cycles_offset = 0;
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

		// DEBUG
		/*if (translate_cycles == 100) {
			printf("BREAKPOINT\n");
			cache->DEBUG_printCacheList();
		}*/

		// Select the right cache to use
		int32_t cache_index = translatorSelectCache();

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

		// for if the dynarec needs to run cache
		if (dynarec_break_loop) {
			dynarec_break_loop = false;
			break;
		}
	} while (translate_cycles % 16 != 0 || jumptbl->checkConditionalCycle() > 0); // Limit a cache update to 16 c8 opcodes at a time, but do not exit if there is a conditional cycle waiting to be updated
}

void Chip8Engine::setKeyState(uint8_t keyindex, KEY_STATE state)
{
	key->setKeyState(keyindex, state);
}

void Chip8Engine::DEBUG_render()
{
	if (getDrawFlag()) {
		//mChip8->DEBUG_printCPUState();
		//mChip8->DEBUG_printSoundTimer();
		if (drawcycles % 16 == 0) DEBUG_renderGFXText();
		setDrawFlag(false);
		drawcycles++;
	}
}

void Chip8Engine::DEBUG_printCPUState()
{
	using namespace std;
	using namespace C8_STATE;
	// Opcode
	cout << "Opcode: " << endl;
	printf("Opcode: 0x%.4x", opcode);
	cout << endl << endl;

	// PC register
	cout << "PC Register: " << endl;
	printf("PC: 0x%.4x", cpu.pc);
	cout << endl << endl;

	// I Register
	cout << "I Register: " << endl;
	printf("I: 0x%.4x", cpu.I);
	cout << endl << endl;

	// V registers
	cout << "V Registers: " << endl;
	for (int i = 0; i < NUM_V_REG; i++) {
		printf("V[%x]: 0x%.2x, ", i, cpu.V[i]);
	}
	cout << endl << "--------------------------" << endl << endl;
	
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

void Chip8Engine::DEBUG_printSoundTimer()
{
	if (timers->getSoundTimer() > 0) printf("\n\n*****BEEP!*****\n\n");
}

void Chip8Engine::DEBUG_printKeyState()
{
	printf("Key States: \n");
	for (int i = 0; i < 0x10; i++) {
		printf("Key[%x]: %u, ", i, key->getKeyState(i));
	}
	printf("\n\n");
}