#include "stdafx.h"

#include "Headers\Globals.h"

#include "Headers\Chip8Engine\Chip8Engine_StackHandler.h"

Chip8Engine_StackHandler::Chip8Engine_StackHandler()
{
	// Register this component in logger
	logger->registerComponent(this);
}

Chip8Engine_StackHandler::~Chip8Engine_StackHandler()
{
	// Deregister this component in logger
	logger->deregisterComponent(this);
}

std::string Chip8Engine_StackHandler::getComponentName()
{
	return std::string("StackHandler");
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
		char buffer[1000];
		sprintf_s(buffer, 1000, "Stack is full! (16 levels). Prepare for a crash or unintended actions! May be a bad rom or emulation bug.");
		logMessage(LOGLEVEL::L_ERROR, buffer);
	}
}

STACK_ENTRY Chip8Engine_StackHandler::getTopStack()
{
	// Decrease stack level by 1
	sp--;
	// Return stack level address
	return stack[sp];
}

#ifdef USE_DEBUG_EXTRA
void Chip8Engine_StackHandler::DEBUG_printStack()
{
	//printf("StackHandler:	Printing stack list (sp = %d):\n", sp);
	for (uint8_t i = 0; i < sp; i++) {
		//printf("			[%d]: c8 return pc = 0x%.4X\n", i, stack[i].c8_address);
	}
	//printf("\n");
}
#endif