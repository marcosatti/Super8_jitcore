#pragma once

#include <string>

#include "Headers\Globals.h"

namespace Chip8Engine {

	class Translator : ILogComponent
	{
	public:
		Translator();
		~Translator();

		std::string getComponentName();

		// This is the translator, which is responsible for converting the Chip8 opcodes into x86 code. This is only invoked once per cache, whenever a PREPARE_FOR_JUMP (or similar) interrupt is handled and an existing cache doesnt exist (ie: CACHE_REGION::x86_pc == 0).

		void emulateTranslatorCycle();
		void emulateTranslatorTimers();
	private:
		// MSN = most significant nibble (half-byte)
		void handleOpcodeMSN_0();
		void handleOpcodeMSN_1();
		void handleOpcodeMSN_2();
		void handleOpcodeMSN_3();
		void handleOpcodeMSN_4();
		void handleOpcodeMSN_5();
		void handleOpcodeMSN_6();
		void handleOpcodeMSN_7();
		void handleOpcodeMSN_8();
		void handleOpcodeMSN_9();
		void handleOpcodeMSN_A();
		void handleOpcodeMSN_B();
		void handleOpcodeMSN_C();
		void handleOpcodeMSN_D();
		void handleOpcodeMSN_E();
		void handleOpcodeMSN_F();
	};

}