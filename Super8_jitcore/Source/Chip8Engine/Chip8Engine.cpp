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
	_snprintf_s(buffer, 1000, "Ran cache ok. Interrupt code = %d (%s).", X86_STATE::x86_interrupt_status_code, X86_STATE::x86_int_status_code_strings[(uint8_t)X86_STATE::x86_interrupt_status_code]);
	logMessage(LOGLEVEL::L_DEBUG, buffer);
#endif

	// Handle Interrupts
	handleInterrupt();

#ifdef USE_DEBUG
	_snprintf_s(buffer, 1000, "New x86_resume_address = 0x%.8X (in cache[%d]).", (uint32_t)X86_STATE::x86_resume_address, cache->findCacheIndexByX86Address(X86_STATE::x86_resume_address));
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
		_snprintf_s(buffer, 1000, "Translator cycle %d, in cache[%d].", translate_cycles, cache->findCacheIndexCurrent());
		logMessage(LOGLEVEL::L_INFO, buffer);
#endif

		// Bounds checking (do not translate outside of rom location)
		if (C8_STATE::cpu.pc > C8_STATE::rom_sz) {
#ifdef USE_VERBOSE
			_snprintf_s(buffer, 1000, "C8 PC was outside of rom location! Exiting translator loop.");
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