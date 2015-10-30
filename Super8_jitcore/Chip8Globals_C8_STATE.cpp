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
		void DEBUG_printC8_STATE()
		{
			printf("C8_STATE: CPU.pc = 0x%.4X, CPU.I = 0x%.4X, opcode = 0x%.4X, &memory = 0x%.8X, &gfxmem = 0x%.8X\n", cpu.pc, cpu.I, opcode, (uint32_t)memory, (uint32_t)gfxmem);
			printf("          V[0] = 0x%.2X, V[1] = 0x%.2X, V[2] = 0x%.2X, V[3] = 0x%.2X, V[4] = 0x%.2X, V[5] = 0x%.2X, V[6] = 0x%.2X, V[7] = 0x%.2X,\n          V[8] = 0x%.2X, V[9] = 0x%.2X, V[A] = 0x%.2X, V[B] = 0x%.2X, V[C] = 0x%.2X, V[D] = 0x%.2X, V[E] = 0x%.2X, V[F] = 0x%.2X\n",
				cpu.V[0], cpu.V[1], cpu.V[2], cpu.V[3], cpu.V[4], cpu.V[5], cpu.V[6], cpu.V[7], cpu.V[8], cpu.V[9], cpu.V[0xA], cpu.V[0xB], cpu.V[0xC], cpu.V[0xD], cpu.V[0xE], cpu.V[0xF]);
		}
	}
}