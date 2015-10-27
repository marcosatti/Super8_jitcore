#pragma once

#include <cstdint>
#include <string>
#include <cstdbool>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "Chip8Globals.h"

////////////////////////////////////////////////////////////////////////////
// Chip8Engine is the entry class into the Chip8 functions and emulation! //
////////////////////////////////////////////////////////////////////////////

using namespace Chip8Globals;

class Chip8Engine {
public:
	Chip8Engine();
	Chip8Engine(uint8_t * _memory, uint8_t * _gfxmem);
	~Chip8Engine();

	void initialise();

	void loadProgram(std::string path);
	void emulationLoop();
	void translatorLoop();
	void handleX86Interrupt();
	int32_t translatorSelectCache();

	void setKeyState(uint8_t keyindex, KEY_STATE state);

	void DEBUG_render();

	void DEBUG_printCPUState();
	void DEBUG_renderGFXText();
	void DEBUG_printSoundTimer();
	void DEBUG_printKeyState();

private:
	
};