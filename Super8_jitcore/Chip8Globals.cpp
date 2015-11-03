#include "stdafx.h"
#include "Chip8Globals.h"

// File contains global vars to simplify use across files.

namespace Chip8Globals {
	Chip8Engine_Interpreter * interpreter;
	Chip8Engine_StackHandler * stack;
	Chip8Engine_Dynarec * dynarec;
	Chip8Engine_CacheHandler * cache;
	Chip8Engine_JumpHandler * jumptbl;
	Chip8Engine_CodeEmitter_x86 * emitter;
	Chip8KeyEngine * key;
	Chip8TimersEngine * timers;

	uint32_t translate_cycles;

	bool drawflag;

	bool getDrawFlag() {
		return drawflag;
	}

	void setDrawFlag(bool isdraw)
	{
		drawflag = isdraw;
	}
}