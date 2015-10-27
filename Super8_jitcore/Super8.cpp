// Super8.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <cstdint>

//#include   // OpenGL graphics and input
#include "Chip8Engine.h"

Chip8Engine * mChip8;

int main(int argc, char **argv) {
	// Set up render system and register input callbacks
	//setupGraphics();
	//setupInput();

	mChip8 = new Chip8Engine();

	// Initialize the Chip8 system and load the game into the memory  
	mChip8->initialise();
	mChip8->loadProgram("..\\chip8roms\\PONG");

	uint32_t count = 0;
	uint8_t randstate = 0;
	for (int i = 0; i < 0x10; i++) {
		randstate = rand() % 0x2;
		mChip8->setKeyState(i, (KEY_STATE)randstate);
	}

	// Emulate
	mChip8->emulationLoop();

	delete mChip8;

	return 0;
}