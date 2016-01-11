#pragma once

#include <string>

#include "Headers\Globals.h"

////////////////////////////////////////////////////////////////////////////
// Chip8Engine is the entry class into the Chip8 functions and emulation! //
////////////////////////////////////////////////////////////////////////////

class Chip8Engine : ILogComponent {
public:
	Chip8Engine();
	~Chip8Engine();

	std::string getComponentName();

	void initialise();
	void loadProgram(std::string path);

	void emulationLoop();
	void translatorLoop();
	void handleInterrupt();

#ifdef USE_DEBUG_EXTRA
	void DEBUG_renderGFXText();
#endif

private:
	void handleInterrupt_PREPARE_FOR_JUMP();
	void handleInterrupt_USE_INTERPRETER();
	void handleInterrupt_OUT_OF_CODE();
	void handleInterrupt_PREPARE_FOR_INDIRECT_JUMP();
	void handleInterrupt_SELF_MODIFYING_CODE();
#ifdef USE_DEBUG_EXTRA
	void handleInterrupt_DEBUG();
#endif
	void handleInterrupt_WAIT_FOR_KEYPRESS();
	void handleInterrupt_PREPARE_FOR_STACK_JUMP();
};