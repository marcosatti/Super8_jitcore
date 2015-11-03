#include "stdafx.h"
#include "Chip8Engine_StackHandler.h"

Chip8Engine_StackHandler::Chip8Engine_StackHandler()
{
}

Chip8Engine_StackHandler::~Chip8Engine_StackHandler()
{
}

void Chip8Engine_StackHandler::setStackLevel(uint8_t level)
{
	sp = level;
}

void Chip8Engine_StackHandler::resetStack()
{
	sp = 0;
	for (int i = 0; i < NUM_STACK_LVLS; i++) {
		stack[i].c8_address = 0x0;
	}
}

void Chip8Engine_StackHandler::setTopStack(STACK_ENTRY entry)
{
	// Make sure stack is not maxed out
	if (sp < 0xF) {
		// Store address in stack
		stack[sp].c8_address = entry.c8_address;
		// Increment stack level by 1
		sp++;
	}
	else {
		printf("StackHandler: ERROR! Stack is maxed out (16)! Stack not modified.");
	}
}

STACK_ENTRY Chip8Engine_StackHandler::getTopStack()
{
	// Decrease stack level by 1
	sp--;
	// Return stack level address
	return stack[sp];
}