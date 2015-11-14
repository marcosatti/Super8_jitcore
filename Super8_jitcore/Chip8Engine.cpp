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
	timers = new Chip8Engine_Timers();
	key = new Chip8Engine_Key();
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
	size_t length = (size_t)file.tellg();
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
	// Handle Interrupts
	handleInterrupt();
	//printf("Chip8Engine: NEW X86_RESUME_ADDRESS = 0x%.8X (in cache[%d])\n", (uint32_t)X86_STATE::x86_resume_address, cache->getCacheIndexByX86Address(X86_STATE::x86_resume_address));
}

void Chip8Engine::handleInterrupt()
{
	switch (X86_STATE::x86_interrupt_status_code) {
	case X86_STATE::PREPARE_FOR_JUMP:
	{
		handleInterrupt_PREPARE_FOR_JUMP();
		break;
	}
	case X86_STATE::USE_INTERPRETER:
	{
		handleInterrupt_USE_INTERPRETER();
		break;
	}
	case X86_STATE::OUT_OF_CODE:
	{
		handleInterrupt_OUT_OF_CODE();
		break;
	}
	case X86_STATE::PREPARE_FOR_INDIRECT_JUMP:
	{
		handleInterrupt_PREPARE_FOR_INDIRECT_JUMP();
		break;
	}
	case X86_STATE::SELF_MODIFYING_CODE:
	{
		handleInterrupt_SELF_MODIFYING_CODE();
		break;
	}
	case X86_STATE::DEBUG:
	{
		handleInterrupt_DEBUG();
		break;
	}
	case X86_STATE::WAIT_FOR_KEYPRESS:
	{
		handleInterrupt_WAIT_FOR_KEYPRESS();
		break;
	}
	case X86_STATE::PREPARE_FOR_STACK_JUMP:
	{
		handleInterrupt_PREPARE_FOR_STACK_JUMP();
		break;
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
#ifdef USE_VERBOSE
			printf("Chip8Engine: Warning: C8 PC was outside of rom location! Running cache again as there is no code to translate (reset pc to 0x0200).\n");
#endif
			C8_STATE::cpu.pc = 0x0200;
			translate_cycles++;
			break;
		}

		// Select the right cache to use & switch
		int32_t cache_index = cache->getCacheWritableByC8PC(C8_STATE::cpu.pc);
		cache->switchCacheByIndex(cache_index);

		// Has code already been compiled? (only valid if x86_pc > 0)
		CACHE_REGION * memory_region = cache->getCacheInfoByIndex(cache_index);
		if (C8_STATE::cpu.pc <= memory_region->c8_end_recompile_pc && memory_region->x86_pc > 0) {
			// This is when we have entered a block that already has compiled code in it... need to switch to end of the region/change region and recompile from there.
			// Update cpu.pc to the end C8 PC of region
			// End C8 PC is defined to be already compiled, so need to plus 2 for next opcode. However, if next opcode is in a different region, we need to select the correct one. Place a continue here so we keep getting the REAL end memory region.
			// When we loop again, the check memory region function will run again and give the correct region.
#ifdef USE_VERBOSE
			printf("Chip8Engine: Warning: C8 PC was not at end of block. Old C8 PC = 0x%.4X, New C8 PC = 0x%.4X. Re-running translator loop to check memory region again.\n", C8_STATE::cpu.pc, cache->getEndC8PCCurrent() + 2);
#endif
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
#ifdef USE_DEBUG
		// DEBUG
		emitter->DYNAREC_EMIT_INTERRUPT(X86_STATE::DEBUG, C8_STATE::opcode, C8_STATE::cpu.pc);
#endif
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