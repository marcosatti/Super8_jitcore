#include "stdafx.h"
#include "Chip8Globals_C8_STATE.h"

#include "Chip8Globals.h"

namespace Chip8Globals {
	namespace C8_STATE {
		C8_CPU cpu;
		uint8_t * memory; // 4096 (0x1000) bytes of memory in total, assumed to be allocated before class initialisation.
		uint8_t * gfxmem; // 2048 (64x32) array containing pixel data (1 or 0) TODO: might be able to change this into a bool array or multiple ints (efficiency)?
		uint16_t opcode; // 16-bit wide opcode holder
		bool drawflag; // Ready to draw screen flag
		uint16_t rom_sz;

		void C8_allocMem() {
			// Allocate main memory and gfx memory
			memory = new uint8_t[MEMORY_SZ];
			gfxmem = new uint8_t[GFX_MEMORY_SZ];
		}

		void C8_clearGFXMem() {
			memset(gfxmem, 0, GFX_MEMORY_SZ);
		}

		void C8_clearMem() {
			memset(memory, 0, MEMORY_SZ);
		}

		void C8_clearRegV() {
			for (int i = 0; i < NUM_V_REG; i++) {
				cpu.V[i] = 0x0;
			}
		}

		void C8_incrementPC(uint8_t bytes)
		{
			// Set region pc to current c8 pc
			cache->setCacheEndC8PCCurrent(cpu.pc);
			// Increments PC by number of bytes specified (and cache)
			cpu.pc += bytes;
		}
		void C8_deallocate()
		{
			delete[] memory;
			delete[] gfxmem;
		}
		uint8_t C8_getPCByteAlignmentOffset(uint16_t c8_pc)
		{
			// PC always starts at 0x0200, and each opcode is ALWAYS 2 bytes long. So if there is a jump to eg: 0x0211, then it has an alignment offset of 1 (alignment of 0 would be 0x0210 or 0x0212). This is used in INVADERS rom.
			// Result will always be 0 or 1 for the C8 specs.
			if (c8_pc % 2 == 0) return 0;
			else return 1;
		}
		void DEBUG_printC8_STATE()
		{
			printf("C8_STATE: &CPU.V = 0x%.8X, &CPU.I = 0x%.8X, &memory = 0x%.8X, &gfxmem = 0x%.8X\n", (uint32_t)cpu.V, (uint32_t)&cpu.I, (uint32_t)memory, (uint32_t)gfxmem);
			printf("          V[0] = 0x%.2X, V[1] = 0x%.2X, V[2] = 0x%.2X, V[3] = 0x%.2X, V[4] = 0x%.2X, V[5] = 0x%.2X, V[6] = 0x%.2X, V[7] = 0x%.2X,\n          V[8] = 0x%.2X, V[9] = 0x%.2X, V[A] = 0x%.2X, V[B] = 0x%.2X, V[C] = 0x%.2X, V[D] = 0x%.2X, V[E] = 0x%.2X, V[F] = 0x%.2X\n          CPU.I = 0x%.4X\n",
				cpu.V[0], cpu.V[1], cpu.V[2], cpu.V[3], cpu.V[4], cpu.V[5], cpu.V[6], cpu.V[7], cpu.V[8], cpu.V[9], cpu.V[0xA], cpu.V[0xB], cpu.V[0xC], cpu.V[0xD], cpu.V[0xE], cpu.V[0xF], cpu.I);
		}
	}
}