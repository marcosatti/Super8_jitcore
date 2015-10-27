#pragma once

#include <cstdint>
#include <iostream>

#define NUM_STACK_LVLS 16 // number of stack levels used (remember: a 'stack' is a LIFO system)

class Chip8StackEngine
{
public:
	Chip8StackEngine();
	~Chip8StackEngine();

	void setStackLevel(uint8_t level);
	void resetStack();
	void setTopStack(uint16_t address);
	uint16_t getTopStack();
private:
	uint16_t stack[NUM_STACK_LVLS]; // Stack used in the implementation, supports 16 levels.
	uint8_t sp; // Stack pointer;
};

