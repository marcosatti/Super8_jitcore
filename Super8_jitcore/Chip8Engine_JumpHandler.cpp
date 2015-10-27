#include "stdafx.h"
#include "Chip8Engine_JumpHandler.h"

using namespace Chip8Globals;

Chip8Engine_JumpHandler::Chip8Engine_JumpHandler()
{
	jump_table.reserve(1024);
	cond_jump_table.reserve(1024);
}


Chip8Engine_JumpHandler::~Chip8Engine_JumpHandler()
{
}

int32_t Chip8Engine_JumpHandler::recordJumpEntry(uint16_t c8_from_, uint16_t c8_to_)
{
	JUMP_ENTRY entry;
	entry.c8_address_from = c8_from_;
	entry.c8_address_to = c8_to_;
	entry.x86_address_to = NULL; // JMP_M_PTR_32 uses this value (unknown at beginning)
	jump_table.push_back(entry);
	printf("JumpHandler: Jump[%d] recorded. C8_from = 0x%.4X, C8_to = 0x%.4X\n", jump_table.size()-1, c8_from_, c8_to_);
	return (jump_table.size() - 1);
}

int32_t Chip8Engine_JumpHandler::recordConditionalJumpEntry(uint16_t c8_from_, uint16_t c8_to_, uint8_t translator_cycles_, uint8_t * x86_address_jump_value_)
{
	COND_JUMP_ENTRY entry;
	entry.c8_address_from = c8_from_;
	entry.c8_address_to = c8_to_;
	entry.x86_address_jump_value = x86_address_jump_value_;
	entry.translator_cycles = translator_cycles_;
	entry.written_flag = 0;
	cond_jump_table.push_back(entry);
	printf("JumpHandler: CONDITIONAL (small) Jump[%d] recorded. C8_from = 0x%.4X, C8_to = 0x%.4X, x86_address = 0x%.8X, cycles = %d\n", cond_jump_table.size() - 1, c8_from_, c8_to_, (uint32_t)x86_address_jump_value_, translator_cycles_);
	return (cond_jump_table.size() - 1);
}

int32_t Chip8Engine_JumpHandler::findConditionalJumpEntry(uint16_t c8_from_, uint16_t c8_to_)
{
	int32_t index = -1;
	for (int32_t i = 0; i < (int32_t)cond_jump_table.size(); i++) {
		if (c8_from_ == cond_jump_table[i].c8_address_from && c8_to_ == cond_jump_table[i].c8_address_to) {
			index = i;
			break;
		}
	}
	return index;
}

void Chip8Engine_JumpHandler::decreaseConditionalCycle()
{
	for (int32_t i = 0; i < (int32_t)cond_jump_table.size(); i++) {
		if (cond_jump_table[i].translator_cycles > 0) {
			cond_jump_table[i].translator_cycles -= 1;
		}
	}
}

void Chip8Engine_JumpHandler::checkAndFillConditionalJumpsByCycles()
{
	for (int32_t i = 0; i < (int32_t)cond_jump_table.size(); i++) {
		if (cond_jump_table[i].translator_cycles == 0 && cond_jump_table[i].written_flag == 0) {
			int8_t relative = (int8_t)(cache->getMemoryRegionCurrentx86Address() - cond_jump_table[i].x86_address_jump_value - 1);
			*(cond_jump_table[i].x86_address_jump_value) = relative;
			cond_jump_table[i].written_flag = 1;
			printf("JumpHandler: CONDITIONAL (small) Jump[%d] found and updated! Value %d written to location 0x%.8X (in cache[%d])\n", i, relative, (uint32_t)cond_jump_table[i].x86_address_jump_value, cache->getMemoryRegionIndexByX86Address(cond_jump_table[i].x86_address_jump_value));
		}
	}
}

int32_t Chip8Engine_JumpHandler::findJumpEntry(uint16_t c8_from_, uint16_t c8_to_)
{
	int32_t index = -1;
	for (int32_t i = 0; i < (int32_t)jump_table.size(); i++) {
		if (c8_from_ == jump_table[i].c8_address_from && c8_to_ == jump_table[i].c8_address_to) {
			index = i;
			break;
		}
	}
	return index;
}

void Chip8Engine_JumpHandler::checkAndFillJumpsByStartC8PC()
{
	for (int32_t i = 0; i < (int32_t)jump_table.size(); i++) {
		for (int32_t j = 0; j < (int32_t)cache->memory_maps.size(); j++) {
			// jump must be to a region start pc, AND the region must not be marked invalid
			if (jump_table[i].c8_address_to == cache->memory_maps[j].c8_start_recompile_pc) { // && cache->memory_maps[j].invalid_flag == 0
				jump_table[i].x86_address_to = cache->getMemoryRegionInfoByIndex(j)->x86_mem_address;
				printf("JumpHandler: Jump[%d] found and updated! C8 Jump 0x%.4X --> 0x%.4X\n", i, jump_table[i].c8_address_from, jump_table[i].c8_address_to);
				printf("             Value 0x%.8X written to x86_address_to (jump to cache[%d])\n", (uint32_t)cache->getMemoryRegionInfoByIndex(j)->x86_mem_address, j);
			}
		}
	}
}

void Chip8Engine_JumpHandler::checkAndFillJumpsByCurrentC8PC()
{
	for (int32_t i = 0; i < (int32_t)jump_table.size(); i++) {
		if (C8_STATE::cpu.pc == jump_table[i].c8_address_to) {
			jump_table[i].x86_address_to = cache->getMemoryRegionCurrentx86Address();
			printf("JumpHandler: Jump[%d] found and updated! C8 Jump 0x%.4X --> 0x%.4X\n", i, jump_table[i].c8_address_from, jump_table[i].c8_address_to);
			printf("             Value 0x%.8X written to x86_address_to (jump to cache[%d])\n", (uint32_t)cache->getMemoryRegionCurrentx86Address(), cache->getMemoryRegionIndex());
		}
	}
}

void Chip8Engine_JumpHandler::invalidateJumpByIndex(int32_t index)
{
	jump_table.erase(jump_table.begin() + index);
}

JUMP_ENTRY * Chip8Engine_JumpHandler::getJumpEntryByIndex(uint32_t index)
{
	return &jump_table[index];
}


