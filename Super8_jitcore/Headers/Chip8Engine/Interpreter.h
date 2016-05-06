#pragma once

#include <cstdint>
#include <string>

#include "Headers\Globals.h"

namespace Chip8Engine {

	class Interpreter : ILogComponent
	{
	public:
		uint16_t opcode;

		Interpreter();
		~Interpreter();

		std::string getComponentName();

		// This is the fallback interpreter, which is run whenever a USE_INTERPRETER interrupt is hit. To use, set the opcode first (setOpcode) then call emulateCycle.
		// The only opcodes that are implemented in here are the opcodes relating to drawing (such as DXY0). They are too time consuming for me to implement in assembly so I have left them in the interpreter.

		void setOpcode(uint16_t c8_opcode);
		void emulateCycle();

	private:
		// emulateCycle will swich and call the related function based on the MSN.
		// MSN = most significant nibble (half-byte). Eg: In DXY0, the MSN = D.
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