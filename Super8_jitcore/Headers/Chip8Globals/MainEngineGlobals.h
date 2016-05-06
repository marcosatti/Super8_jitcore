#pragma once

#include <cstdint>

// Forward declarations to stop circular references.
namespace Chip8Engine {
	class JumpHandler;
	class Interpreter;
	class Translator;
	class Timers;
	class CodeEmitter_x86;
	class CacheHandler;
	class Key;
	class StackHandler;
}

namespace Chip8Globals {
	namespace MainEngineGlobals {
		// Declare global components needed for this emulator. 
		// Most of the engine components below are referenced from other components, so it is easier to keep a global variable list.
		// These are allocated when MainEngine::intitialise() is called.
		extern Chip8Engine::Interpreter * interpreter;
		extern Chip8Engine::StackHandler * stackhandler;
		extern Chip8Engine::Translator * translator;
		extern Chip8Engine::CacheHandler * cachehandler;
		extern Chip8Engine::JumpHandler * jumphandler;
		extern Chip8Engine::CodeEmitter_x86 * codeemitter_x86;
		extern Chip8Engine::Key * key;
		extern Chip8Engine::Timers * timers;

		// Used for interfacing with the OS, through the SDL library. Whenever the draw flag is set, SDL will update the window displayed (if turned on).
		extern bool drawflag;
		extern bool getDrawFlag();
		extern void setDrawFlag(bool isdraw);
	}
}