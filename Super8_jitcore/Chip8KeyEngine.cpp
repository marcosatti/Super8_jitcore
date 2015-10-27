#include "stdafx.h"
#include "Chip8KeyEngine.h"


Chip8KeyEngine::Chip8KeyEngine()
{
	// Set all key states to 0.
	memset(key, 0, NUM_KEYS);
}


Chip8KeyEngine::~Chip8KeyEngine()
{
}

void Chip8KeyEngine::setKeyState(uint8_t keyindex, KEY_STATE state)
{
	key[keyindex] = (uint8_t) state;
}

uint8_t Chip8KeyEngine::getKeyState(uint8_t keyindex)
{
	return key[keyindex]; // 0 = key is up, 1 = key is down.
}
