#include "stdafx.h"
#include "Chip8StackEngine.h"


Chip8StackEngine::Chip8StackEngine()
{
}


Chip8StackEngine::~Chip8StackEngine()
{
}

void Chip8StackEngine::setStackLevel(uint8_t level)
{
	sp = level;
}

void Chip8StackEngine::resetStack()
{
	sp = 0;
	for (int i = 0; i < NUM_STACK_LVLS; i++) {
		stack[i] = 0x0;
	}
}

void Chip8StackEngine::setTopStack(uint16_t address)
{
	// Make sure stack is not maxed out
	if (sp < 0xF) {
		// Store address in stack
		stack[sp] = address;
		// Increment stack level by 1
		sp++;
	}
	else {
		std::cout << "ERROR! Stack is maxed out (16)! Stack not modified.";
	}
}

uint16_t Chip8StackEngine::getTopStack()
{
	// Decrease stack level by 1
	sp--;
	// Return stack level address
	return stack[sp];
}
