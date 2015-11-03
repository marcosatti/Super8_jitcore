#pragma once

#include <cstdint>
#include <cstdlib>

#define NUM_KEYS 0x10

enum KEY_STATE {
	UP = 0,
	DOWN = 1
};

class Chip8KeyEngine
{
public:
	uint8_t X86_KEY_PRESSED;
	uint8_t key[16]; // Array to store 0 -> F key states.

	Chip8KeyEngine();
	~Chip8KeyEngine();
	void setKeyState(uint8_t keyindex, KEY_STATE state);
	uint8_t getKeyState(uint8_t keyindex);
private:
	
};