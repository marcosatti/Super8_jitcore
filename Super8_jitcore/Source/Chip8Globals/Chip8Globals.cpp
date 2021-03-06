#include "stdafx.h"

#include <cstdint>

#include "Headers\Globals.h"

// File contains global vars to simplify use across files.

// Forward decl's
class Chip8Engine_JumpHandler;
class Chip8Engine_Interpreter;
class Chip8Engine_Dynarec;
class Chip8Engine_Timers;
class Chip8Engine_CodeEmitter_x86;
class Chip8Engine_CacheHandler;
class Chip8Engine_Key;
class Chip8Engine_StackHandler;
#ifdef USE_SDL_GRAPHICS
struct SDL_Texture;
#endif

namespace Chip8Globals {
	Chip8Engine_Interpreter * interpreter;
	Chip8Engine_StackHandler * stack;
	Chip8Engine_Dynarec * dynarec;
	Chip8Engine_CacheHandler * cache;
	Chip8Engine_JumpHandler * jumptbl;
	Chip8Engine_CodeEmitter_x86 * emitter;
	Chip8Engine_Key * key;
	Chip8Engine_Timers * timers;

	uint32_t translate_cycles;

	bool drawflag;

	bool getDrawFlag() {
		return drawflag;
	}

	void setDrawFlag(bool isdraw) {
		drawflag = isdraw;
	}

#ifdef USE_SDL_GRAPHICS
	SDL_Texture * SDL_texture;
	uint32_t * SDL_gfxmem;
	int SDL_pitch;
#endif
}