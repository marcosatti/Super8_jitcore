#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>

#define NUM_BITS_PER_BYTE 8
#define NUM_V_REG 16 // 16 8-bit data registers from V0 -> VF
#define MEMORY_SZ 4096 // 4K of RAM (0x0000 -> 0x0FFF accessable)
#define GFX_MEMORY_SZ 2048 // 2K of VRAM (64 x 32)
#define GFX_XRES 64
#define GFX_YRES 32

struct C8_CPU {
	uint16_t pc; // 16-bit wide program counter, contains current address being executed
	uint8_t V[NUM_V_REG]; // 16 8-bit data registers, named from V0 -> VF, VF doubles as a carry flag
	uint16_t I; // 16-bit Address register, used with opcodes that involve memory operations
};

namespace Chip8Globals {
	namespace C8_STATE {
		extern struct C8_CPU cpu;
		extern uint8_t * memory;
		extern uint8_t * gfxmem;
		extern uint16_t opcode;

		extern void C8_allocMem();
		extern void C8_clearMem();
		extern void C8_clearGFXMem();
		extern void C8_clearRegV();
		extern void C8_incrementPC(uint8_t bytes = 2); // 2 Bytes by default
		extern void C8_deallocate();
		extern inline uint8_t C8_getPCByteAlignmentOffset(uint16_t c8_pc); // PC always starts at 0x0200, and each opcode is always 2 bytes long. So if there is a jump to eg: 0x0211, then it has an alignment offset of 1 (alignment of 0 would be 0x0210 or 0x0212).

		// Note that sprites are 8 - bits wide, while fonts are 4 - bits, so the other 4-bits at the end are padded 0xN0 (0's).
		// This is to be copied into the Chip8's memory at runtime.
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
#define FONT_WIDTH 5
#define FONTSET_SZ (sizeof(Chip8Globals::C8_STATE::chip8_fontset)/sizeof(Chip8Globals::C8_STATE::chip8_fontset[0]))

		extern uint16_t rom_sz;

#ifdef USE_DEBUG
		extern void DEBUG_printC8_STATE();
#endif
	}
}