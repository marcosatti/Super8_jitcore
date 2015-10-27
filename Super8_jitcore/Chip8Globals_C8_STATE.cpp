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
			cache->setMemoryRegionC8EndPC(cpu.pc);
			// Increments PC by number of bytes specified (and cache)
			cpu.pc += bytes;
		}
		void C8_deallocate()
		{
			delete[] memory;
			delete[] gfxmem;
		}
	}
}