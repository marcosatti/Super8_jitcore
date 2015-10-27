#include "stdafx.h"
#include "Chip8TimersEngine.h"

using namespace Chip8Globals;

Chip8TimersEngine::Chip8TimersEngine()
{
}


Chip8TimersEngine::~Chip8TimersEngine()
{
}

void Chip8TimersEngine::updateTimers_Interpreter()
{
	// Update timers
	if (delay_timer > 0) delay_timer--;

	if (sound_timer > 0) {
		// if (sound_timer == 1) (std::cout << "BEEP!\n"); // TODO: implement beep
		sound_timer--;
	}
}

void Chip8TimersEngine::setSoundTimer(uint8_t num)
{
	// Set sound timer to count from
	sound_timer = num;
}

void Chip8TimersEngine::setDelayTimer(uint8_t num)
{
	// Set delay timer to count from
	delay_timer = num;
}

uint8_t Chip8TimersEngine::getSoundTimer()
{
	return sound_timer;
}

uint8_t Chip8TimersEngine::getDelayTimer()
{
	return delay_timer;
}
