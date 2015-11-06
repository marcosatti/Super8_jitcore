#pragma once

#include <cstdint>

#include "Chip8Globals_C8_STATE.h"
#include "Chip8Globals_X86_STATE.h"

#include "Chip8Engine_Interpreter.h"
#include "Chip8Engine_Dynarec.h"
#include "Chip8Engine_CacheHandler.h"
#include "Chip8Engine_Timers.h"
#include "Chip8Engine_StackHandler.h"
#include "Chip8Engine_JumpHandler.h"
#include "Chip8Engine_Key.h"

//#define USE_DEBUG

#ifdef USE_DEBUG_EXTRA
#define USE_DEBUG
#endif

// Forward decl's
class Chip8Engine_JumpHandler;
class Chip8Engine_Interpreter;
class Chip8Engine_Dynarec;
class Chip8Engine_Timers;
class Chip8Engine_CodeEmitter_x86;
class Chip8Engine_CacheHandler;

namespace Chip8Globals {
	// Declare Globals for C8
	extern Chip8Engine_Interpreter * interpreter;
	extern Chip8Engine_StackHandler * stack;
	extern Chip8Engine_Dynarec * dynarec;
	extern Chip8Engine_CacheHandler * cache;
	extern Chip8Engine_JumpHandler * jumptbl;
	extern Chip8Engine_CodeEmitter_x86 * emitter;
	extern Chip8Engine_Key * key;
	extern Chip8Engine_Timers * timers;

	extern uint32_t translate_cycles;

	extern bool drawflag;
	extern bool getDrawFlag();
	extern void setDrawFlag(bool isdraw);
}