#pragma once

#include <cstdint>
#include <string>

#include "Headers\Globals.h"

class Chip8Engine_Interpreter : ILogComponent
{
public:
	uint16_t opcode;

	Chip8Engine_Interpreter();
	~Chip8Engine_Interpreter();

	std::string getComponentName();

	void setOpcode(uint16_t c8_opcode);
	void emulateCycle();

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
private:
};