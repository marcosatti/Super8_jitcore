#pragma once

#include <cstdint>
#include <cstdlib>
#include <string>

#include "../Chip8Globals/Chip8Globals.h"

#define NUM_KEYS 0x10

enum KEY_STATE {
	UP = 0,
	DOWN = 1
};

class Chip8Engine_Key : ILogComponent
{
public:
	uint8_t X86_KEY_PRESSED;
	uint8_t key[16]; // Array to store 0 -> F key states.

	Chip8Engine_Key();
	~Chip8Engine_Key();

	std::string getComponentName();

	void setKeyState(uint8_t keyindex, KEY_STATE state);
	uint8_t getKeyState(uint8_t keyindex);
private:
};