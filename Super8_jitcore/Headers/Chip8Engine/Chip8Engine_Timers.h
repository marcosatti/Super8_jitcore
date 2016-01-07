#pragma once

#include <cstdint>
#include <iostream>

#include "../Chip8Globals/Chip8Globals.h"

class Chip8Engine_Timers {
public:
	uint8_t delay_timer; // A timer register that counts down to zero at 60Hz
	uint8_t sound_timer; // A sound timer regsiter that runs at 60Hz, and will emit a sound when it hits zero.

	Chip8Engine_Timers();
	~Chip8Engine_Timers();
private:
};
