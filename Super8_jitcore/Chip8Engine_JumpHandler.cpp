#include "stdafx.h"
#include "Chip8Engine_JumpHandler.h"

using namespace Chip8Globals;

Chip8Engine_JumpHandler::Chip8Engine_JumpHandler()
{
	jump_list.reserve(1024);
	cond_jump_list.reserve(1024);
}


Chip8Engine_JumpHandler::~Chip8Engine_JumpHandler()
{
}

int32_t Chip8Engine_JumpHandler::recordJumpEntry(uint16_t c8_to_)
{
	JUMP_ENTRY entry;
	entry.c8_address_to = c8_to_;
	entry.x86_address_to = NULL; // JMP_M_PTR_32 uses this value (unknown at beginning)
	jump_list.push_back(entry);
	printf("JumpHandler: Jump[%d] recorded. C8_to = 0x%.4X\n", jump_list.size()-1, c8_to_);
	return (jump_list.size() - 1);
}

int32_t Chip8Engine_JumpHandler::recordConditionalJumpEntry(uint16_t c8_from_, uint16_t c8_to_, uint8_t translator_cycles_, uint8_t * x86_address_jump_value_)
{
	COND_JUMP_ENTRY entry;
	entry.c8_address_from = c8_from_;
	entry.c8_address_to = c8_to_;
	entry.x86_address_jump_value = x86_address_jump_value_;
	entry.translator_cycles = translator_cycles_;
	entry.written_flag = 0;
	cond_jump_list.push_back(entry);
	printf("JumpHandler: CONDITIONAL (small) Jump[%d] recorded. C8_from = 0x%.4X, C8_to = 0x%.4X, x86_address = 0x%.8X, cycles = %d\n", cond_jump_list.size() - 1, c8_from_, c8_to_, (uint32_t)x86_address_jump_value_, translator_cycles_);
	return (cond_jump_list.size() - 1);
}

int32_t Chip8Engine_JumpHandler::findConditionalJumpEntry(uint16_t c8_from_, uint16_t c8_to_)
{
	int32_t index = -1;
	for (int32_t i = 0; i < (int32_t)cond_jump_list.size(); i++) {
		if (c8_from_ == cond_jump_list[i].c8_address_from && c8_to_ == cond_jump_list[i].c8_address_to) {
			index = i;
			break;
		}
	}
	return index;
}

void Chip8Engine_JumpHandler::decreaseConditionalCycle()
{
	for (int32_t i = 0; i < (int32_t)cond_jump_list.size(); i++) {
		if (cond_jump_list[i].translator_cycles > 0) {
			cond_jump_list[i].translator_cycles -= 1;
		}
	}
}

uint8_t Chip8Engine_JumpHandler::checkConditionalCycle()
{
	for (int32_t i = 0; i < (int32_t)cond_jump_list.size(); i++) {
		uint8_t cycles = cond_jump_list[i].translator_cycles;
		if (cycles > 0) return cycles;
	}
	return 0;
}

void Chip8Engine_JumpHandler::checkAndFillConditionalJumpsByCycles()
{
	for (int32_t i = 0; i < (int32_t)cond_jump_list.size(); i++) {
		if (cond_jump_list[i].translator_cycles == 0 && cond_jump_list[i].written_flag == 0) {
			int8_t relative = (int8_t)(cache->getEndX86AddressCurrent() - cond_jump_list[i].x86_address_jump_value - 1);
			*(cond_jump_list[i].x86_address_jump_value) = relative;
			cond_jump_list[i].written_flag = 1;
			printf("JumpHandler: CONDITIONAL (small) Jump[%d] found and updated! Value %d written to location 0x%.8X (in cache[%d])\n", i, relative, (uint32_t)cond_jump_list[i].x86_address_jump_value, cache->getCacheIndexByX86Address(cond_jump_list[i].x86_address_jump_value));
		}
	}
}

int32_t Chip8Engine_JumpHandler::findJumpEntry(uint16_t c8_to_)
{
	int32_t index = -1;
	for (int32_t i = 0; i < (int32_t)jump_list.size(); i++) {
		if (c8_to_ == jump_list[i].c8_address_to) {
			index = i;
			break;
		}
	}
	return index;
}

void Chip8Engine_JumpHandler::checkAndFillJumpsByStartC8PC()
{
	for (int32_t i = 0; i < (int32_t)jump_list.size(); i++) {
		// Jump cache handling done by CacheHandler, so this function just updates the jump table locations
		CACHE_REGION * region = NULL;
		int32_t index = cache->allocNewCacheByJumpC8PC(jump_list[i].c8_address_to);
		region = cache->getCacheInfoByIndex(index);
		jump_list[i].x86_address_to = region->x86_mem_address;
		//printf("JumpHandler: C8 Jump[%d] Updated: 0x%.4X--> 0x%.4X. Address 0x%.8X written to x86_address_to (jump to cache[%d])\n", i, jump_list[i].c8_address_from, jump_list[i].c8_address_to, (uint32_t)region->x86_mem_address, index);
	}
}

void Chip8Engine_JumpHandler::invalidateJumpByIndex(int32_t index)
{
	jump_list.erase(jump_list.begin() + index);
}

JUMP_ENTRY * Chip8Engine_JumpHandler::getJumpEntryByIndex(uint32_t index)
{
	return &jump_list[index];
}

void Chip8Engine_JumpHandler::DEBUG_printJumpList()
{
	for (uint32_t i = 0; i < jump_list.size(); i++) {
		printf("JumpHandler: Jump[%d] c8_address_to = 0x%.4X, x86_address_to = 0x%.8X\n",
			i, jump_list[i].c8_address_to, (uint32_t)jump_list[i].x86_address_to);
	}
}

void Chip8Engine_JumpHandler::DEBUG_printCondJumpList()
{
	for (uint32_t i = 0; i < cond_jump_list.size(); i++) {
		printf("JumpHandler: CondJump[%d]: c8_address_from = 0x%.4X, c8_address_to = 0x%.4X, x86_address_jump_value = 0x%.8X, ",
			i, cond_jump_list[i].c8_address_from, cond_jump_list[i].c8_address_to, (uint32_t)cond_jump_list[i].x86_address_jump_value);
		printf("translator_cycles = %d, written_flag = %d\n",
			cond_jump_list[i].translator_cycles, cond_jump_list[i].written_flag);
	}
}
