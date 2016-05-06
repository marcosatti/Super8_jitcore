#pragma once

#include <cstdint>
#include <string>

#include "Headers\Globals.h"

#define NUM_KEYS 0x10

// A simple class to hold the keypad states.

namespace Chip8Engine {

	enum KEY_STATE {
		UP = 0,
		DOWN = 1
	};

	class Key : ILogComponent
	{
	public:
		uint8_t X86_KEY_PRESSED; // Used by the WAIT_FOR_KEYPRESS interrupt, which writes a key that has been pressed into this value. It is then read by the translated code and handled by the rom (check the opcode FX0A in Translator.cpp).

		Key();
		~Key();

		std::string getComponentName();

		void setKeyState(uint8_t keyindex, KEY_STATE state);
		KEY_STATE getKeyState(uint8_t keyindex);
		void clearKeyState();

		// Not to be accessed directly! Opcodes such as EXA1 need to access this directly from assembly however (see Translator.cpp).
		uint8_t key[16]; // Array to store 0 -> F key states.

	private:
		
	};

}