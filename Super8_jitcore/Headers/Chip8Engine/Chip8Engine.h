#pragma once

#include <cstdint>
#include <string>
#include <cstdbool>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "../Chip8Globals/Chip8Globals.h"
#include "../Chip8Engine/Chip8Engine_CacheHandler.h"
#include "../Chip8Engine/Chip8Engine_JumpHandler.h"
#include "../Chip8Engine/Chip8Engine_Interpreter.h"
#include "../Chip8Engine/Chip8Engine_CodeEmitter_x86.h"
#include "../Chip8Engine/Chip8Engine_Key.h"
#include "../Chip8Engine/Chip8Engine_StackHandler.h"
#include "../Chip8Engine/Chip8Engine_Timers.h"
#include "../Chip8Engine/Chip8Engine_Dynarec.h"

////////////////////////////////////////////////////////////////////////////
// Chip8Engine is the entry class into the Chip8 functions and emulation! //
////////////////////////////////////////////////////////////////////////////

using namespace Chip8Globals;

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