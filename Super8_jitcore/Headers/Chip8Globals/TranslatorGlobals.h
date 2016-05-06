#pragma once

#include <cstdint>

namespace Chip8Globals {
	namespace TranslatorGlobals {
		// Used to relay information from the Translator class to the translator loop if a jump was encountered, and therefore translating should stop.
		extern bool block_finished;

		// A verbose variable to display how many cycles (opcodes) the translator loop has run for in total.
		extern uint32_t translate_cycles;
	}
}