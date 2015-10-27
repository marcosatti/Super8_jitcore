#pragma once

#include "Chip8Globals.h"

#include "Chip8Engine_CodeEmitter_x86.h"
#include "Chip8Engine_StackHandler.h"

class Chip8Engine_Dynarec {
public:
	Chip8Engine_Dynarec();
	~Chip8Engine_Dynarec();

	void initialiseFirstMemoryRegion();

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