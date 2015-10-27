#include "stdafx.h"
#include "Chip8Engine_JumpTable.h"

using namespace Chip8Globals;

Chip8Engine_JumpTable::Chip8Engine_JumpTable()
{
	table.reserve(0xFF); // Default size of 255 should be more than enough.
	resetC8Stack();
}

Chip8Engine_JumpTable::~Chip8Engine_JumpTable()
{
}

void Chip8Engine_JumpTable::handleRelativeJump(uint16_t c8_relative_offset, uint8_t jmp_num_bytes)
{
	// Records a new jump using:
	// uint8 * x86_jump_address, location of the relative jump offset we need to fill in later
	// uint16 c8_current_address, location of the current jump instruction, needed to calculate the c8_absolute_address
	// uint16 c8_relative_offset, location of the jump offset from the current c8 pc, needed to calculate the c8_absolute_address
	// uint16 c8_jump_location = c8_current_address + c8_relative_offset
	handleAbsoluteJump(cpu.pc + c8_relative_offset, jmp_num_bytes);
}

void Chip8Engine_JumpTable::handleAbsoluteJump(uint16_t c8_to_absolute_address, uint8_t jmp_num_bytes)
{
	// Records a new jump using:
	// uint8 * x86_jump_address, location of the relative jump offset we need to fill in later
	// uint16 c8_absolute_address, location of c8 jump address
	// uint16 c8_jump_location = c8_absolute_address
	JUMP_TABLE_ENTRY entry;
	entry.x86_jump_from_address = dynarec->cache->cache_mem + dynarec->cache->cache_pc - jmp_num_bytes - 1;
	entry.x86_jump_to_address = NULL;
	entry.c8_jump_from_address = cpu.pc;
	entry.c8_jump_to_address = c8_to_absolute_address;
	entry.jump_num_bytes = jmp_num_bytes;

	table.push_back(entry);
	printf("jmptable: New jmp recorded. Will later fill mem location 0x%.8X with a location.\n", (uint32_t)entry.x86_jump_from_address);
	printf("          Current C8 PC = 0x%.4X, looking for match when C8 PC = 0x%.4X\n", cpu.pc, c8_to_absolute_address);
	printf("          New table size = %d\n", table.size());
}

uint32_t Chip8Engine_JumpTable::checkAndFillJumps()
{
	// Checks if c8_pc == c8_jump_location, and if so, fill in the x86 jump location with the x86 offset calculated
	// Returns the number of jumps filled in
	printf("jmptable: Checking for unhandled jumps to fill in... (table size = %d)\n", table.size());
	uint8_t * x86_current_address = dynarec->cache->cache_mem + dynarec->cache->cache_pc;
	uint32_t count = 0;
	for (uint32_t i = 0; i < table.size(); i++) {
		if (table[i].x86_jump_to_address == NULL) {
			// WIN32 specific
			table[i].x86_jump_to_address = (uint8_t *)VirtualAlloc(0, 0xFFF, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			// Setup dynarec with new addresses and translate opcode
			dynarec->cache->cache_mem = table[i].x86_jump_to_address;
			dynarec->cache->cache_pc = 0;
			cpu.pc = table[i].c8_jump_to_address;
			dynarec->emulateTranslatorCycle();
			//int32_t relative = (x86_current_address - table[i].x86_jump_from_address + table[i].jump_num_bytes - 1);
			printf("jmptable: Match found. Filling mem location 0x%.8X with value 0x%X (%d) (bytes = %d)\n", (uint32_t)table[i].x86_jump_from_address, (uint32_t)relative, relative, table[i].jump_num_bytes);
			if (table[i].jump_num_bytes == 4) *((uint32_t*)table[i].x86_jump_from_address) = (int32_t)relative; // 32 Bit jmp relative
			else if (table[i].jump_num_bytes == 2) *((uint16_t*)table[i].x86_jump_from_address) = (int16_t)relative; // 16 Bit jmp relative
			else *((uint8_t*)table[i].x86_jump_from_address) = (int8_t)relative; // 8 Bit jmp relative
			count++;
			printf("jmptable: Successfully filled in x86 jump to address in table[%d]\n", i);
		}
	}
	return count;
}

bool Chip8Engine_JumpTable::isJumpsUnhandled()
{
	// Checks to see if any jumps are unhandled in the table still (need to recompile code first before the cache is allowed to run)
	bool unhandled = false;
	for (uint32_t i = 0; i < table.size(); i++) {
		(table[i].x86_jump_to_address == NULL) ? (unhandled = true) : (unhandled = false);
	}
	return unhandled;
}

void Chip8Engine_JumpTable::resetC8Stack()
{
	C8_sp = 0;
	for (int i = 0; i < NUM_TRANSLATOR_STACK_LVLS; i++) {
		C8_stack[i] = 0x0;
	}
	
}

void Chip8Engine_JumpTable::setC8TopStack(uint8_t * address)
{
	// Make sure stack is not maxed out
	if (C8_sp < 0xF) {
		// Store address in stack
		C8_stack[C8_sp] = address;
		// Increment stack level by 1
		C8_sp++;
	}
	else {
		std::cout << "jmptable: ERROR! Dynarec Stack is maxed out (16)! Stack not modified.\n";
	}
}

uint8_t * Chip8Engine_JumpTable::getC8TopStack()
{
	// Decrease stack level by 1
	C8_sp--;
	// Return stack level address
	return C8_stack[C8_sp];
}
