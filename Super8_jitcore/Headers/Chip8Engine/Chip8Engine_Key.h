#pragma once

#include <cstdint>
#include <string>

#define NUM_KEYS 0x10

enum KEY_STATE {
	UP = 0,
	DOWN = 1
};

class Chip8Engine_Key : ILogComponent
{
public:
	uint8_t X86_KEY_PRESSED;

	Chip8Engine_Key();
	~Chip8Engine_Key();

	std::string getComponentName();

	void setKeyState(uint8_t keyindex, KEY_STATE state);
	KEY_STATE getKeyState(uint8_t keyindex);
	void clearKeyState();

	// ONLY FOR Chip8Engine TO ACCESS DIRECTLY! USE ABOVE FUNCTIONS FOR GENERAL PURPOSE USES.
	uint8_t key[16]; // Array to store 0 -> F key states.

private:

};