#include "stdafx.h"

#include <cstdint>

#include "Headers\Chip8Globals\MainEngineGlobals.h"

// See the corresponding header file MainEngineGlobals.h for descriptions.

namespace Chip8Globals {
	namespace MainEngineGlobals {
		Chip8Engine::Interpreter * interpreter;
		Chip8Engine::StackHandler * stackhandler;
		Chip8Engine::Translator * translator;
		Chip8Engine::CacheHandler * cachehandler;
		Chip8Engine::JumpHandler * jumphandler;
		Chip8Engine::CodeEmitter_x86 * codeemitter_x86;
		Chip8Engine::Key * key;
		Chip8Engine::Timers * timers;

		bool drawflag;

		bool getDrawFlag() {
			return drawflag;
		}

		void setDrawFlag(bool isdraw)
		{
			drawflag = isdraw;
		}
	}
}