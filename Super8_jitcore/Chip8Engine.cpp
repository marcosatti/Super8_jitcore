#include "stdafx.h"
#include "Chip8Engine.h"

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

	// Get length of file.
	file.seekg(0, std::ios::end);
	size_t length = (size_t) file.tellg();
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
		cache->execCache_CDECL();
		printf("Chip8Engine: Ran cache ok. NEW X86_RESUME_ADDRESS = 0x%.8X (in cache[%d])\n", (uint32_t)X86_STATE::x86_resume_address, cache->getMemoryRegionIndexByX86Address(X86_STATE::x86_resume_address));
		printf("             Interrupt code = %d, (optional) C8 handle opcode = 0x%.4X\n", X86_STATE::x86_status_code, X86_STATE::x86_resume_c8_pc);
		handleX86Interrupt();
	}
}

void Chip8Engine::handleX86Interrupt()
{
	switch (X86_STATE::x86_status_code) {
	case X86_STATE::PREPARE_FOR_JUMP:
	{
		// ! ! ! X86_STATE::x86_resume_c8_pc contains jump location ! ! !

		// Perform a check to make sure the region is available (otherwise a runtime error will be thrown due to jump to unknown address)
		// If the jump is into the middle of a region, need to invalidate and create a new region
		//int32_t index = cache->checkMemoryRegionAllocatedByC8PC(X86_STATE::x86_resume_c8_pc);
		//if (index == -1) cache->allocNewMemoryRegionByC8PC(X86_STATE::x86_resume_c8_pc);
		//else {
		//	// Cache found, but check if its to the start of a region
		//	if (X86_STATE::x86_resume_c8_pc != cache->getMemoryRegionInfoByIndex(index)->c8_start_recompile_pc) {
		//		cache->invalidateMemoryRegionByIndex(index);
		//		cache->allocNewMemoryRegionByC8PC(X86_STATE::x86_resume_c8_pc);
		//	}
		//}
		// Need to update the jump table/cache before the jumps are made.
		jumptbl->checkAndFillJumpsByStartC8PC();
		break;
	}
	case X86_STATE::USE_INTERPRETER:
	{
		// ! ! ! X86_STATE::x86_resume_c8_pc contains interpreter opcode ! ! !
		// Opcode hasnt been implemented in the dynarec yet, need to use interpreter
		interpreter->setOpcode(X86_STATE::x86_resume_c8_pc);
		interpreter->emulateCycle();
		DEBUG_render();
		break;
	}
	case X86_STATE::OUT_OF_CODE:
	{
		// ! ! ! X86_STATE::x86_resume_c8_pc contains start pc of cache ! ! !
		// End of cache was reached, usually because there was no jump back to another cache
		// Set c8 pc to end of memory region c8 pc, so it will start recompiling code from there
		// Flush caches that are marked after caches have run
		cache->invalidateAllMemoryRegionsByFlag();

		// Set pc to recompile code from and x86_resume_addr which will be at the beginning of the next instruction emitted
		MEM_REGION * region = cache->getMemoryRegionInfoByC8PC(X86_STATE::x86_resume_c8_pc);
		C8_STATE::cpu.pc = region->c8_end_recompile_pc;
		X86_STATE::x86_resume_address = region->x86_mem_address + region->x86_pc;

		// Conditional case: even though a region ends with a jump, sometimes it can go over the jump code, if there is a conditional jump
		// Need to check for this by checking against the do not write flag, and emit a jump to the next region
		if (region->stop_write_flag) {
			// First record jump in jump table if DNE (so it will get updated on every translator loop)
			int32_t tblindex = jumptbl->findJumpEntry(region->c8_end_recompile_pc + 2, region->c8_end_recompile_pc + 2);
			if (tblindex == -1) {
				tblindex = jumptbl->recordJumpEntry(region->c8_end_recompile_pc + 2, region->c8_end_recompile_pc + 2);
			}

			// Check if the jump location is not to start of a cache, and if so, need to mark invalid
			// I think this will never be true? Cache from will have do not write, so the next pc must be in a cache that has the correct starting pc?
			int32_t index = cache->checkMemoryRegionAllocatedByC8PC(region->c8_end_recompile_pc + 2);
			if (index == -1) cache->allocNewMemoryRegionByC8PC(region->c8_end_recompile_pc + 2); // No cache was found, alloc a new one
			else {
				// Cache found, but check if its to the start of a region
				if ((region->c8_end_recompile_pc + 2) != cache->getMemoryRegionInfoByIndex(index)->c8_start_recompile_pc) {
					cache->setInvalidFlagByIndex(index, 1);
					cache->allocNewMemoryRegionByC8PC(region->c8_end_recompile_pc + 2);
				}
			}

			// Emit jump (to out of code cache)
			int32_t old_index = cache->getMemoryRegionIndex();
			cache->switchMemoryRegionByC8PC(X86_STATE::x86_resume_c8_pc);
			emitter->DYNAREC_EMIT_INTERRUPT(X86_STATE::PREPARE_FOR_JUMP, region->c8_end_recompile_pc + 2);
			emitter->JMP_M_PTR_32((uint32_t*)&jumptbl->jump_table[tblindex].x86_address_to);
			cache->switchMemoryRegionByIndex(old_index);
		}

		// Start recompiling code
		translatorLoop();
		break;
	}
	case X86_STATE::PREPARE_FOR_INDIRECT_JUMP:
	{
		break;
	}
	case X86_STATE::SELF_MODIFYING_CODE:
	{
		break;
	}
	}
}

int32_t Chip8Engine::translatorSelectCache()
{
	// First check if memory region is ready/allocated (check for both current pc (indicates already recompiled), and check for pc-2 (ready to write to))
	int32_t cache_index = cache->checkMemoryRegionAllocatedByC8PC(C8_STATE::cpu.pc);
	if (cache_index != -1) {
		// Cache exists for current PC
		// Check if its invalid
		if (cache->getInvalidFlagByIndex(cache_index)) {
			// Cache was invalid, so create a new cache at current PC
			cache_index = cache->allocAndSwitchNewMemoryRegionByC8PC(C8_STATE::cpu.pc);
		}
		else {
			// Cache was not marked as invalid, so next check for the do not write flag.
			// However, it does not matter as later the pc will be moved to the end of the region (code must already exist in this logic block), so just switch to this cache for now
			cache->switchMemoryRegionByIndex(cache_index);
		}
	}
	else {
		// Cache does not exist for current PC, so it hasnt been compiled before. Need to get region which is available to hold this recompiled code
		// Check for cache with PC - 2 end PC.
		cache_index = cache->checkMemoryRegionAllocatedByC8PC(C8_STATE::cpu.pc - 2);
		if (cache_index != -1) {
			// Cache exists for current PC - 2
			// Check if its invalid
			if (cache->getInvalidFlagByIndex(cache_index)) {
				// Cache was invalid, so create a new cache at current PC
				cache_index = cache->allocAndSwitchNewMemoryRegionByC8PC(C8_STATE::cpu.pc);
			}
			else {
				// Cache was not marked as invalid, so next check for the do not write flag.
				if (cache->getStopWriteMemoryRegionByIndex(cache_index)) {
					// Cache has do not write flag set, so allocate a new region
					cache_index = cache->allocAndSwitchNewMemoryRegionByC8PC(C8_STATE::cpu.pc);
				}
				else {
					// Cache does not have do not write flag set, so its safe to write to this cache
					cache->switchMemoryRegionByIndex(cache_index);
				}
			}
		}
		else {
			// No cache was found for PC - 2 as well, so allocate a new one for current pc
			cache_index = cache->allocAndSwitchNewMemoryRegionByC8PC(C8_STATE::cpu.pc);
		}
	}
	return cache_index;
}

void Chip8Engine::translatorLoop()
{
	do {
		printf("\nChip8Engine: Running translator cycle: %d\n", translate_cycles);

		// Select the right cache to use
		int32_t cache_index = translatorSelectCache();

		// Has code already been compiled? (only valid if start != end c8 pc)
		MEM_REGION * memory_region = cache->getMemoryRegionInfoByIndex(cache_index);
		if (C8_STATE::cpu.pc <= memory_region->c8_end_recompile_pc &&
			memory_region->c8_start_recompile_pc != memory_region->c8_end_recompile_pc) {
			// This is when we have entered a block that already has compiled code in it... need to switch to end of the region/change region and recompile from there.
			// Update cpu.pc to the end C8 PC of region
			// End C8 PC is defined to be already compiled, so need to plus 2 for next opcode. However, if next opcode is in a different region, we need to select the correct one. Place a continue here so we keep getting the REAL end memory region.
			// When we loop again, the check memory region function will run again and give the correct region.
			C8_STATE::cpu.pc = cache->getMemoryRegionC8EndPC() + 2;
			printf("Chip8Engine: Warning: C8 PC was not at end of block. Old C8 PC = 0x%.4X, New C8 PC = 0x%.4X. Re-running emulation loop to check memory region again.\n", C8_STATE::cpu.pc - 2, C8_STATE::cpu.pc);
			// Update cycle number
			translate_cycles++;
			continue;
		}

		// We are now in a valid memory region and at the end of the block (ready to recompile again)
		// Fetch Opcode
		C8_STATE::opcode = C8_STATE::memory[C8_STATE::cpu.pc] << 8 | C8_STATE::memory[C8_STATE::cpu.pc + 1]; // We have 8-bit memory, but an opcode is 16-bits long. Need to construct opcode from 2 successive memory locations.

		// DEBUG
		printf("             Mem Region = %d, x86 Mem Region Start = 0x%.8X, x86 Mem Region PC = 0x%.8X, invalid_flag = %d\n", cache->getMemoryRegionIndex(), (uint32_t)memory_region->x86_mem_address, (uint32_t)memory_region->x86_pc, memory_region->invalid_flag);
		printf("             x86 RESUME ADDRESS = 0x%.8X (in cache[%d])\n", (uint32_t)X86_STATE::x86_resume_address, cache->getMemoryRegionIndexByX86Address(X86_STATE::x86_resume_address));
		printf("             C8 PC = 0x%.4X, C8 Opcode = 0x%.4X, Mem Region START C8 PC = 0x%.4X, Mem Region END C8 PC = 0x%.4X\n", C8_STATE::cpu.pc, C8_STATE::opcode, memory_region->c8_start_recompile_pc, memory_region->c8_end_recompile_pc);

		// Update Timers
		dynarec->emulateTranslatorTimers();
		// Translate
		dynarec->emulateTranslatorCycle();
		// Check and fill in conditional jumps & decrease num of cycles
		jumptbl->checkAndFillConditionalJumpsByCycles();
		jumptbl->decreaseConditionalCycle();

		// Update cycle number
		translate_cycles++;

		// for if the dynarec needs to run cache
		if (dynarec_break_loop) {
			dynarec_break_loop = false;
			break;
		}
	} while (translate_cycles % 16 != 0); // Limit a cache update to 16 c8 opcodes at a time
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
		DEBUG_renderGFXText();
		setDrawFlag(false);
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