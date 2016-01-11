#include "stdafx.h"

#include "Headers\Globals.h"

#include "Headers\Chip8Engine\Chip8Engine_Key.h"

Chip8Engine_Key::Chip8Engine_Key()
{
	// Set all key states to 0.
	memset(key, 0, NUM_KEYS);

	// Register this component in logger
	logger->registerComponent(this);
}

Chip8Engine_Key::~Chip8Engine_Key()
{
	// Deregister this component in logger
	logger->deregisterComponent(this);
}

std::string Chip8Engine_Key::getComponentName()
{
	return std::string("Key");
}

void Chip8Engine_Key::setKeyState(uint8_t keyindex, KEY_STATE state)
{
	key[keyindex] = (uint8_t)state;
}

uint8_t Chip8Engine_Key::getKeyState(uint8_t keyindex)
{
	return key[keyindex]; // 0 = key is up, 1 = key is down.
}