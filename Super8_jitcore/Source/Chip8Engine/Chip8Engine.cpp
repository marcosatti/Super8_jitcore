#include "stdafx.h"

#include <cstdint>
#include <fstream>

#include <SDL.h>
#ifdef _WIN32
#include <Windows.h>
#pragma comment(lib, "winmm.lib")
#endif

#include "Headers\Globals.h"

#include "Headers\Chip8Globals\Chip8Globals.h"
#include "Headers\Chip8Engine\Chip8Engine.h"
#include "Headers\Chip8Engine\Chip8Engine_CacheHandler.h"
#include "Headers\Chip8Engine\Chip8Engine_CodeEmitter_x86.h"
#include "Headers\Chip8Engine\Chip8Engine_Dynarec.h"
#include "Headers\Chip8Engine\Chip8Engine_Interpreter.h"
#include "Headers\Chip8Engine\Chip8Engine_JumpHandler.h"
#include "Headers\Chip8Engine\Chip8Engine_Key.h"
#include "Headers\Chip8Engine\Chip8Engine_StackHandler.h"
#include "Headers\Chip8Engine\Chip8Engine_Timers.h"

using namespace Chip8Globals;

// Variables
#ifdef LIMIT_SPEED_BY_DRAW_CALLS
uint32_t old_ticks = 0;
uint32_t new_ticks = 0;
int32_t delta_ticks = 0;
const uint32_t limiter_max_time_slice = (1000 / TARGET_FRAMES_PER_SECOND);
#endif
#ifdef LIMIT_SPEED_BY_INSTRUCTIONS
const uint32_t limiter_delay_ms = (1000 / TARGET_CPU_SPEED_HZ);
#endif

Chip8Engine::Chip8Engine() {
	// Register this component in logger
	logger->registerComponent(this);

#ifdef LIMITER_ON
#ifdef _WIN32
	timeBeginPeriod(1);
#endif
#endif
}

Chip8Engine::~Chip8Engine() {
	// Deregister this component in logger
	logger->deregisterComponent(this);

#ifdef LIMITER_ON
#ifdef _WIN32
	timeEndPeriod(1);
#endif
#endif

	delete key;
	delete stack;
	delete timers;
	delete interpreter;
	delete dynarec;
	delete emitter;
	delete cache;
	delete jumptbl;
}

std::string Chip8Engine::getComponentName()
{
	return std::string("Chip8Engine");
}

void Chip8Engine::initialise() {
	// Initialise & reset timers
	timers = new Chip8Engine_Timers();
	key = new Chip8Engine_Key();
	interpreter = new Chip8Engine_Interpreter();
	emitter = new Chip8Engine_CodeEmitter_x86();
	dynarec = new Chip8Engine_Dynarec();
	cache = new Chip8Engine_CacheHandler();
	stack = new Chip8Engine_StackHandler();
	jumptbl = new Chip8Engine_JumpHandler();

	logger->updateFormat();

	translate_cycles = 0;

	C8_STATE::C8_allocMem();
	C8_STATE::cpu.pc = (uint16_t)0x200;					// Program counter starts at 0x200
	C8_STATE::opcode = (uint16_t)0x0000;				// Reset current opcode
	C8_STATE::cpu.I = (uint16_t)0x000;					// Reset index register
	stack->resetStack();								// Reset stack pointer
#ifndef USE_SDL_GRAPHICS
	C8_STATE::C8_clearGFXMem();							// Clear display
#endif
	C8_STATE::C8_clearRegV();							// Clear registers V0-VF
	C8_STATE::C8_clearMem();							// Clear memory

	// Load fontset
	memcpy(C8_STATE::memory, C8_STATE::chip8_fontset, FONTSET_SZ);

	// initiate timers
	SDL_Thread * timersthread = SDL_CreateThread(timers->runThread_Timers, "TimersThread", timers);
	if (timersthread == nullptr) {
		logMessage(LOGLEVEL::L_FATAL, "Could not create timers thread!"); 
		exit(4);
	}

	// Setup/update cache here pop/push etc
	cache->setupCache_CDECL();

	// Setup first memory region
	cache->initFirstCache();
}

void Chip8Engine::loadProgram(std::string path) {
	// Load whole program into memory, starting at address 0x200.
	// Open file.
	std::ifstream file(path, std::ios::in | std::ios::binary);

	// Get length of file, store end location in global var rom_sz
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

#ifdef USE_DEBUG
	char buffer[1000];
	sprintf_s(buffer, 1000, "Ran cache ok. Interrupt code = %d (%s).", X86_STATE::x86_interrupt_status_code, X86_STATE::x86_int_status_code_strings[(uint8_t)X86_STATE::x86_interrupt_status_code]);
	logMessage(LOGLEVEL::L_DEBUG, buffer);
#endif

	// Handle Interrupts
	handleInterrupt();

#ifdef USE_DEBUG
	sprintf_s(buffer, 1000, "New x86_resume_address = 0x%.8X (in cache[%d]).", (uint32_t)X86_STATE::x86_resume_address, cache->findCacheIndexByX86Address(X86_STATE::x86_resume_address));
	logMessage(LOGLEVEL::L_DEBUG, buffer);
#endif
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
#ifdef USE_DEBUG_EXTRA
	case X86_STATE::DEBUG:
	{
		handleInterrupt_DEBUG();
		break;
	}
#endif
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
	case X86_STATE::UPDATE_TIMERS:
	{
		handleInterrupt_UPDATE_TIMERS();
		break;
	}
#ifdef LIMIT_SPEED_BY_INSTRUCTIONS
	case X86_STATE::DELAY_INSTRUCTION:
	{
		handleInterrupt_DELAY_INSTRUCTION();
		break;
	}
#endif
	default:
	{
		logMessage(LOGLEVEL::L_ERROR, "DEFAULT CASE REACHED IN handleInterrupt. SOMETHING IS WRONG!");
		break;
	}
	}
}

void Chip8Engine::translatorLoop()
{
	// Set the loop condition to false first, which will become true when a jump is encountered and then break the loop.
	Dynarec::block_finished = false;

	// Translator loop.
	while (Dynarec::block_finished == false) { // Limit a cache update to blocks of code.
#ifdef USE_VERBOSE
		char buffer[1000];
#endif
#ifdef USE_DEBUG
		// Print number of translator cycles parsed.
		sprintf_s(buffer, 1000, "Translator cycle %d, in cache[%d].", translate_cycles, cache->findCacheIndexCurrent());
		logMessage(LOGLEVEL::L_INFO, buffer);
#endif

		// Bounds checking (do not translate outside of rom location). Undefined results if this is reached, since it should in theory never get beyond the end of the rom.
		if (C8_STATE::cpu.pc > C8_STATE::rom_sz) {
#ifdef USE_VERBOSE
			sprintf_s(buffer, 1000, "C8 PC was outside of rom location! Exiting translator loop.");
			logMessage(LOGLEVEL::L_WARNING, buffer);
#endif
			C8_STATE::cpu.pc = 0x0200;
			Dynarec::block_finished = true;
			translate_cycles++;
			break;
		}

		// Fetch Opcode
		C8_STATE::opcode = C8_STATE::memory[C8_STATE::cpu.pc] << 8 | C8_STATE::memory[C8_STATE::cpu.pc + 1]; // We have 8-bit memory, but an opcode is 16-bits long. Need to construct opcode from 2 successive memory locations.

		// Update Timers
		//dynarec->emulateTranslatorTimers();

#ifdef USE_DEBUG_EXTRA
		// DEBUG
		emitter->DYNAREC_EMIT_INTERRUPT(X86_STATE::DEBUG, C8_STATE::opcode, C8_STATE::cpu.pc);
#endif
		// Translate
		dynarec->emulateTranslatorCycle();

		// Check and fill in conditional jumps & decrease num of cycles
		jumptbl->decreaseConditionalCycle();
		jumptbl->checkAndFillConditionalJumpsByCycles();

		// Delay instruction by 1000/TARGET_CPU_SPEED_HZ if limiter option is on.
#ifdef LIMIT_SPEED_BY_INSTRUCTIONS
		emitter->DYNAREC_EMIT_INTERRUPT(X86_STATE::DELAY_INSTRUCTION);
#endif

		// Update cycle number
		translate_cycles++;
	} 
}

void Chip8Engine::handleInterrupt_PREPARE_FOR_JUMP()
{
	// ! ! ! X86_STATE::x86_interrupt_c8_param1 contains jump location ! ! !
	// Flush caches that are marked
	cache->invalidateCacheByFlag();

	// Need to update the jump table/cache before the jumps are made.
#ifdef USE_DEBUG
	cache->DEBUG_printCacheList();
#endif
	jumptbl->checkAndFillJumpsByStartC8PC();
}

void Chip8Engine::handleInterrupt_USE_INTERPRETER()
{
	// ! ! ! X86_STATE::x86_interrupt_c8_param1 contains interpreter opcode ! ! !

	// Opcode hasnt been implemented in the dynarec yet, need to use interpreter
	interpreter->setOpcode(X86_STATE::x86_interrupt_c8_param1);
	interpreter->emulateCycle();

#ifdef LIMIT_SPEED_BY_DRAW_CALLS
	// If defined, attempts to delay emulation by ((uint)1000/TARGET_FRAMES_PER_SECOND - execution time since last draw call)ms.
	new_ticks = SDL_GetTicks();
	delta_ticks = limiter_max_time_slice - (new_ticks - old_ticks);
	if (delta_ticks > 0) SDL_Delay(delta_ticks);
	old_ticks = SDL_GetTicks();
#endif
}

void Chip8Engine::handleInterrupt_OUT_OF_CODE()
{
	// ! ! ! X86_STATE::x86_interrupt_c8_param1 contains start pc of cache, X86_STATE::x86_interrupt_x86_param1 contains starting x86 address of cache ! ! !

	// Get cache details that caused interrupt.
	int32_t cache_index = cache->findCacheIndexByX86Address(X86_STATE::x86_interrupt_x86_param1); // param1 should be the base address of the cache, so we can find the cache that interrupted by searching for this value.
	CACHE_REGION * region = cache->getCacheInfoByIndex(cache_index);
	// Remember to reset the entry point to the current cache PC.
	X86_STATE::x86_resume_address = region->x86_mem_address + region->x86_pc;

	// Select cache in CacheHandler equal to cache that caused interrupt
	cache->switchCacheByIndex(cache_index);

	// Case 1 - cache is out of code (empty) and needs recompiling code.
	if (region->x86_pc == 0) {
		// Start recompiling code in blocks
		C8_STATE::cpu.pc = region->c8_start_recompile_pc;
		translatorLoop();
	}
	// Case 2 - cache has code, but needs a jump needs to happen into the next cache (end pc + 2). This is due to a conditional jump.
	else {
		// First make sure jump table entry
		int32_t tblindex = jumptbl->getJumpIndexByC8PC(region->c8_end_recompile_pc + 2);
		// Emit the jump
		emitter->DYNAREC_EMIT_INTERRUPT(X86_STATE::PREPARE_FOR_JUMP, region->c8_end_recompile_pc + 2);
		emitter->JMP_M_PTR_32((uint32_t*)&jumptbl->getJumpInfoByIndex(tblindex)->x86_address_to);
	}
}

void Chip8Engine::handleInterrupt_PREPARE_FOR_INDIRECT_JUMP()
{
	// ! ! ! X86_STATE::x86_interrupt_c8_param1 contains opcode ! ! !
	// Flush caches that are marked
	cache->invalidateCacheByFlag();

	// Need to update the jump table/cache before the jumps are made.
	//cache->DEBUG_printCacheList();
	switch (X86_STATE::x86_interrupt_c8_param1 & 0xF000) {
	case 0xB000:
	{
		uint16_t c8_address = X86_STATE::x86_interrupt_c8_param1 & 0x0FFF;
		c8_address += C8_STATE::cpu.V[0]; // get address to jump to

										  // Jump cache handling done by CacheHandler, so this function just updates the jump table locations
		int32_t cache_index = cache->getCacheWritableByStartC8PC(c8_address);
		CACHE_REGION * region = cache->getCacheInfoByIndex(cache_index);
		jumptbl->x86_indirect_jump_address = region->x86_mem_address;
	}
	default:
	{
		logMessage(LOGLEVEL::L_ERROR, "DEFAULT CASE REACHED IN PREPARE_FOR_INDIRECT_JUMP. SOMETHING IS WRONG!");
	}
	}
}

void Chip8Engine::handleInterrupt_SELF_MODIFYING_CODE()
{
	// ! ! ! X86_STATE::x86_interrupt_c8_param1 contains opcode from translator ! ! !

	// Only 2 opcodes in the C8 specs that do this. For SMC, need to invalidate cache that the memory writes to
	switch (X86_STATE::x86_interrupt_c8_param1 & 0xF0FF) {
	case 0xF033:
	{
		// 0xFX33: Splits the decimal representation of Vx into 3 locations: hundreds stored in address I, tens in address I+1, and ones in I+2.
		//cache->DEBUG_printCacheList();
		//uint16_t I = C8_STATE::cpu.I;
		cache->setInvalidFlagByC8PC(C8_STATE::cpu.I);
		cache->setInvalidFlagByC8PC(C8_STATE::cpu.I + 1);
		cache->setInvalidFlagByC8PC(C8_STATE::cpu.I + 2);
		break;
	}
	case 0xF055:
	{
		// 0xFX55: Copies all current values in registers V0 -> Vx to memory starting at address I.
		uint8_t vx = (X86_STATE::x86_interrupt_c8_param1 & 0x0F00) >> 8; // Need to bit shift by 8 to get to a single base16 digit.
		for (uint8_t i = 0; i <= vx; i++) {
			cache->setInvalidFlagByC8PC(C8_STATE::cpu.I + i);
		}
		break;
	}
	}
}

#ifdef USE_DEBUG_EXTRA
void Chip8Engine::handleInterrupt_DEBUG()
{
	char buffer[1000];
	sprintf_s(buffer, 1000, "!!! Debug Interrupt, Opcode = 0x%.4X, C8PC = 0x%.4X !!!", X86_STATE::x86_interrupt_c8_param1, X86_STATE::x86_interrupt_c8_param2);
	logMessage(LOGLEVEL::L_DEBUG, buffer);
	C8_STATE::DEBUG_printC8_STATE();
	//X86_STATE::DEBUG_printX86_STATE();
	//cache->DEBUG_printCacheList();
	//jumptbl->DEBUG_printJumpList();
	//jumptbl->DEBUG_printCondJumpList();
}
#endif

void Chip8Engine::handleInterrupt_WAIT_FOR_KEYPRESS()
{
	// ! ! ! X86_STATE::x86_interrupt_c8_param1 contains the C8 opcode ! ! !
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
}

void Chip8Engine::handleInterrupt_PREPARE_FOR_STACK_JUMP()
{
	// ! ! ! X86_STATE::x86_interrupt_c8_param1 contains either: 0x2NNN (call, address = NNN) or 0x00EE (ret), X86_STATE::x86_interrupt_c8_param2 contains the return address for an 0x2000 call ! ! !
	// Flush caches that are marked
	cache->invalidateCacheByFlag();

	switch (X86_STATE::x86_interrupt_c8_param1 & 0xF000) {
	case 0x2000:
	{
		// get jump location & return location
		uint16_t jump_c8_pc = X86_STATE::x86_interrupt_c8_param1 & 0x0FFF;
		uint16_t return_c8_pc = X86_STATE::x86_interrupt_c8_param2;

		// Record stack entry for the return point - which will be the next opcode!
		STACK_ENTRY entry;
		entry.c8_address = return_c8_pc;
		stack->setTopStack(entry);

		// First get jump table entry
		int32_t tblindex = jumptbl->getJumpIndexByC8PC(jump_c8_pc);

		// Need to check/alloc jump location caches
		jumptbl->checkAndFillJumpsByStartC8PC();

		// Set stack->x86_address_to equal to jumptable location
		stack->x86_address_to = jumptbl->getJumpInfoByIndex(tblindex)->x86_address_to;
		break;
	}
	case 0x0000:
	{
		// Get stack entry & set jump location
		STACK_ENTRY entry = stack->getTopStack();

		// First get jump table entry
		int32_t tblindex = jumptbl->getJumpIndexByC8PC(entry.c8_address);

		// Need to check/alloc jump location caches
		jumptbl->checkAndFillJumpsByStartC8PC();

		// Set stack->x86_address_to equal to jumptable location
		stack->x86_address_to = jumptbl->getJumpInfoByIndex(tblindex)->x86_address_to;
		break;
	}
	default:
	{
		logMessage(LOGLEVEL::L_ERROR, "DEFAULT CASE REACHED IN PREPARE_FOR_STACK_JUMP. SOMETHING IS WRONG!");
		break;
	}
	}
}

void Chip8Engine::handleInterrupt_UPDATE_TIMERS()
{
	switch (X86_STATE::x86_interrupt_c8_param1 & 0xF0FF)
	{
	case 0xF007:
	{
		// 0xFX07: Sets Vx to the value of the delay timer.
		// TODO: check if correct.
		uint8_t vx = (X86_STATE::x86_interrupt_c8_param1 & 0x0F00) >> 8;
		timers->TIMERS_SPIN_LOCK();
		C8_STATE::cpu.V[vx] = timers->getDelayTimer();
		timers->TIMERS_SPIN_UNLOCK();
		break;
	}
	case 0xF015:
	{
		// 0xFX15: Sets the delay timer to Vx.
		// TODO: check if correct.
		uint8_t vx = (X86_STATE::x86_interrupt_c8_param1 & 0x0F00) >> 8;
		timers->TIMERS_SPIN_LOCK();
		timers->setDelayTimer(C8_STATE::cpu.V[vx]);
		timers->TIMERS_SPIN_UNLOCK();
		break;
	}
	case 0xF018:
	{
		// 0xFX18: Sets the sound timer to Vx.
		// TODO: check if correct.
		uint8_t vx = (X86_STATE::x86_interrupt_c8_param1 & 0x0F00) >> 8;
		timers->TIMERS_SPIN_LOCK();
		timers->setSoundTimer(C8_STATE::cpu.V[vx]);
		timers->TIMERS_SPIN_UNLOCK();
		break;
	}
	default:
	{
		logMessage(LOGLEVEL::L_ERROR, "DEFAULT CASE REACHED IN UPDATE_TIMERS. SOMETHING IS WRONG!");
		break;
	}
	}
}

#ifdef LIMIT_SPEED_BY_INSTRUCTIONS
void Chip8Engine::handleInterrupt_DELAY_INSTRUCTION()
{
	SDL_Delay(limiter_delay_ms);
}
#endif

#ifdef USE_DEBUG_EXTRA
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
#endif