#pragma once

#include <cstdint>

#include "Headers\Globals.h"

// Defines relating to Chip8 constants.
#define NUM_BITS_PER_BYTE 8 // 8 bits per byte, used in graphics functions within Interpreter.
#define NUM_V_REG 16 // 16 8-bit data registers from V0 -> VF
#define MEMORY_SZ 4096 // 4K of RAM (0x0000 -> 0x0FFF accessable)
#define GFX_MEMORY_SZ 2048 // 2K of VRAM (64 x 32)
#define GFX_XRES 64 // X-res of 64
#define GFX_YRES 32 // Y-res of 32

namespace Chip8Globals {
	namespace C8_STATE {

		struct C8_CPU {
			uint16_t pc; // 16-bit wide program counter, contains current address being executed
			uint8_t V[NUM_V_REG]; // 16 8-bit data registers, named from V0 -> VF, VF doubles as a carry flag
			uint16_t I; // 16-bit Address register, used with opcodes that involve memory operations
		};

		extern struct C8_CPU cpu; // The Chip8 CPU struct, used throughout the emulator. Results from the dynamic recompiled code also gets written here.
		extern uint8_t * memory; // The Chip8 main memory, used thoughout the emulator. Sometimes it is accessed directly from the dynamic recompiled code (write operations), but this means the corresponding cache's need to be recreated.
		extern uint8_t * gfxmem; // The Chip8 graphics memory, used in the graphics functions. Will not be used if the USE_SDL flag is defined, as graphics read and writes work directly on the SDL buffer instead.
		extern uint16_t opcode; // A temporary variable used to construct a 16-bit opcode from 2 8-bit memory locations.

		extern void C8_allocMem(); // Used to allocate the main and graphics memory (on the heap) at initialisation.
		extern void C8_clearMem(); // Used to set all of the memory to zero's at initialisation. 
		extern void C8_clearGFXMem(); // Used to set all of the graphics memory to zero's at initialisation and when the 00E0 opcode is run. Will not be invoked if the USE_SDL flag is defined, as SDL uses its own buffer.
		extern void C8_clearRegV(); // Used to clear the Chip8 registers at initialisation.
		extern void C8_incrementPC(uint8_t bytes = 2); // 2 Bytes by default
		extern void C8_deallocate(); // Used at program exit in order to clear the allocated heap memory.

		// This is the Chip8 fontset to be copied into the Chip8's memory at runtime.
		// Note that sprites are 8 - bits wide, while fonts are 4 - bits, so the other 4-bits at the end are padded 0xN0 (0's).
		const uint8_t chip8_fontset[] = {
			0xF0, 0x90, 0x90, 0x90, 0xF0,		// 0
			0x20, 0x60, 0x20, 0x20, 0x70,		// 1
			0xF0, 0x10, 0xF0, 0x80, 0xF0,		// 2
			0xF0, 0x10, 0xF0, 0x10, 0xF0,		// 3
			0x90, 0x90, 0xF0, 0x10, 0x10,		// 4
			0xF0, 0x80, 0xF0, 0x10, 0xF0,		// 5
			0xF0, 0x80, 0xF0, 0x90, 0xF0,		// 6
			0xF0, 0x10, 0x20, 0x40, 0x40,		// 7
			0xF0, 0x90, 0xF0, 0x90, 0xF0,		// 8
			0xF0, 0x90, 0xF0, 0x10, 0xF0,		// 9
			0xF0, 0x90, 0xF0, 0x90, 0x90,		// A
			0xE0, 0x90, 0xE0, 0x90, 0xE0,		// B
			0xF0, 0x80, 0x80, 0x80, 0xF0,		// C
			0xE0, 0x90, 0x90, 0x90, 0xE0,		// D
			0xF0, 0x80, 0xF0, 0x80, 0xF0,		// E
			0xF0, 0x80, 0xF0, 0x80, 0x80		// F
		};
#define FONT_WIDTH 5 // A fontset character is defined by 5 bytes.
#define FONTSET_SZ (sizeof(Chip8Globals::C8_STATE::chip8_fontset)/sizeof(Chip8Globals::C8_STATE::chip8_fontset[0])) // Used when copying the fontset into the Chip8 main memory.

		extern uint16_t rom_sz; // Used when a rom is loaded to store the number of bytes it occupies. This is useful for determining if the translator loop will go out of bounds.

#ifdef USE_DEBUG
		extern void DEBUG_printC8_STATE();
#endif
	}
}