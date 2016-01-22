#pragma once

#include <cstdint>
#include <string>

#include "Headers\Globals.h"

class Chip8Engine_Timers : ILogComponent
{
public:
	uint8_t delay_timer; // A timer register that counts down to zero at 60Hz.
	uint8_t sound_timer; // A sound timer register that runs at 60Hz, and will emit a sound when it hits zero.

	std::string getComponentName();

	std::string getComponentName();

	Chip8Engine_Timers();
	~Chip8Engine_Timers();
private:
};
