#pragma once

#include <cstdint>
#include <iostream>

#include "Chip8Globals.h"

class Chip8TimersEngine {
public:
	uint8_t delay_timer; // A timer register that counts down to zero at 60Hz
	uint8_t sound_timer; // A sound timer regsiter that runs at 60Hz, and will emit a sound when it hits zero.

	Chip8TimersEngine();
	~Chip8TimersEngine();

	void updateTimers_Interpreter();
	void setSoundTimer(uint8_t num);
	void setDelayTimer(uint8_t num);
	uint8_t getSoundTimer();
	uint8_t getDelayTimer();
private:
	
};

