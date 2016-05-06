#pragma once

#include <string>

#include "Headers\Globals.h"

////////////////////////////////////////////////////////////////////////////
// Chip8Engine is the entry class into the Chip8 functions and emulation! //
////////////////////////////////////////////////////////////////////////////

namespace Chip8Engine {

	class MainEngine : ILogComponent {
	public:
		MainEngine();
		~MainEngine();

		std::string getComponentName();

		void initialise(std::string rom_path); // Gets the Chip8/x86 system ready and loads rom into 0x200.

		void emulationLoop(); // Starts executing the emulator, with 1. executing the cache code and 2. handling the interrupt generated. This is run in a loop by the main function.
		void translatorLoop(); // Translator loop which is invoked whenever a cache needs x86 code generated. This converts the Chip8 code into x86 assembly.
		void handleInterrupt(); // Function which handles the interrupts that are generated, by switching and calling one of the sub functions below.

	private:
		// Interrupt handlers.
		void handleInterrupt_PREPARE_FOR_JUMP();
		void handleInterrupt_USE_INTERPRETER();
		void handleInterrupt_OUT_OF_CODE();
		void handleInterrupt_PREPARE_FOR_INDIRECT_JUMP();
		void handleInterrupt_SELF_MODIFYING_CODE();
#ifdef USE_DEBUG_EXTRA
		void handleInterrupt_DEBUG(); // Used for debugging, will print to console the opcode that was translated and the register states.
#endif
		void handleInterrupt_WAIT_FOR_KEYPRESS();
		void handleInterrupt_PREPARE_FOR_STACK_JUMP();

#ifdef USE_DEBUG_EXTRA
		void DEBUG_renderGFXText();
#endif
	};

}