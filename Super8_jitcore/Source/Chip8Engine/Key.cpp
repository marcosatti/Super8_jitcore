#include "stdafx.h"

#include "Headers\Globals.h"

#include "Headers\Chip8Engine\Key.h"

using namespace Chip8Engine;

Key::Key()
{
	// Set all key states to 0.
	memset(key, 0, NUM_KEYS);

	// Register this component in logger
	logger->registerComponent(this);
}

Key::~Key()
{
	// Deregister this component in logger
	logger->deregisterComponent(this);
}

std::string Key::getComponentName()
{
	return std::string("Key");
}

void Key::setKeyState(uint8_t keyindex, KEY_STATE state)
{
	key[keyindex] = (uint8_t)state;
}

KEY_STATE Key::getKeyState(uint8_t keyindex)
{
	return (KEY_STATE)key[keyindex]; // 0 = key is up, 1 = key is down.
}

void Key::clearKeyState()
{
	memset(key, 0, 16);
}
