#include "stdafx.h"
#include "Chip8Engine_JumpHandler.h"

using namespace Chip8Globals;

Chip8Engine_JumpHandler::Chip8Engine_JumpHandler() : jump_list(1024), cond_jump_list(1024)
{
}

Chip8Engine_JumpHandler::~Chip8Engine_JumpHandler()
{
}

int32_t Chip8Engine_JumpHandler::recordJumpEntry(uint16_t c8_to_)
{
	JUMP_ENTRY entry;
	entry.c8_address_to = c8_to_;
	entry.x86_address_to = NULL; // JMP_M_PTR_32 uses this value (unknown at beginning)
	entry.filled_flag = 0;
	jump_list.push_back(entry);
	printf("JumpHandler: Jump[%d] recorded. C8_to = 0x%.4X\n", jump_list.size() - 1, c8_to_);
	return (jump_list.size() - 1);
}

int32_t Chip8Engine_JumpHandler::recordConditionalJumpEntry(uint16_t c8_from_, uint16_t c8_to_, uint8_t translator_cycles_, uint32_t * x86_address_jump_value_)
{
	COND_JUMP_ENTRY entry;
	entry.c8_address_from = c8_from_;
	entry.c8_address_to = c8_to_;
	entry.x86_address_jump_value = x86_address_jump_value_;
	entry.translator_cycles = translator_cycles_;
	cond_jump_list.push_back(entry);
	//printf("JumpHandler: CONDITIONAL (small) Jump[%d] recorded. C8_from = 0x%.4X, C8_to = 0x%.4X, x86_address = 0x%.8X, cycles = %d\n", cond_jump_list.size() - 1, c8_from_, c8_to_, (uint32_t)x86_address_jump_value_, translator_cycles_);
	return (cond_jump_list.size() - 1);
}

void Chip8Engine_JumpHandler::decreaseConditionalCycle()
{
	for (int32_t i = 0; i < (int32_t)cond_jump_list.size(); i++) {
		if (cond_jump_list[i]->translator_cycles > 0) {
			cond_jump_list[i]->translator_cycles -= 1;
		}
	}
}

uint8_t Chip8Engine_JumpHandler::checkConditionalCycle()
{
	for (int32_t i = 0; i < (int32_t)cond_jump_list.size(); i++) {
		uint8_t cycles = cond_jump_list[i]->translator_cycles;
		if (cycles > 0) return cycles;
	}
	return 0;
}

void Chip8Engine_JumpHandler::checkAndFillConditionalJumpsByCycles()
{
	int32_t list_sz = cond_jump_list.size();
	for (int32_t i = 0; i < list_sz; i++) {
		if (cond_jump_list[i]->translator_cycles == 0) {
			int32_t relative = (int32_t)((uint32_t)cache->getEndX86AddressCurrent() - (uint32_t)cond_jump_list[i]->x86_address_jump_value - sizeof(uint32_t)); // 4 is size of uint32_t, as eip is at the end of the jump instruction but we calculate the relative size based on the start address of the relative
			*(cond_jump_list[i]->x86_address_jump_value) = relative;

			// remove entry after its been filled
			cond_jump_list.remove(i);
			list_sz = cond_jump_list.size(); // update list size again
			i -= 1; // decrease i by 1 so it rechecks the current i'th value in the list (which would have been i+1 if there was no remove).

			//printf("JumpHandler: CONDITIONAL (small) Jump[%d] found and updated! Value %d written to location 0x%.8X (in cache[%d])\n", i, relative, (uint32_t)cond_jump_list[i]->x86_address_jump_value, cache->findCacheIndexCurrent());
		}
	}
}

int32_t Chip8Engine_JumpHandler::findJumpEntry(uint16_t c8_to_)
{
	int32_t index = -1;
	for (int32_t i = 0; i < (int32_t)jump_list.size(); i++) {
		if (c8_to_ == jump_list[i]->c8_address_to) {
			index = i;
			break;
		}
	}
	return index;
}

void Chip8Engine_JumpHandler::checkAndFillJumpsByStartC8PC()
{
	CACHE_REGION * region = NULL;
	for (int32_t i = 0; i < (int32_t)jump_list.size(); i++) {
		if (jump_list[i]->filled_flag == 0) {
			// Jump cache handling done by CacheHandler, so this function just updates the jump table locations
			int32_t index = cache->getCacheWritableByStartC8PC(jump_list[i]->c8_address_to);
			region = cache->getCacheInfoByIndex(index);
			jump_list[i]->x86_address_to = region->x86_mem_address;
			printf("JumpHandler: C8 Jump[%d] Updated: 0x%.4X. Address 0x%.8X written to x86_address_to (jump to cache[%d])\n", i, jump_list[i]->c8_address_to, (uint32_t)region->x86_mem_address, index);
			jump_list[i]->filled_flag = 1;
		}
	}
}

void Chip8Engine_JumpHandler::invalidateJumpByIndex(int32_t index)
{
	jump_list.remove(index);
}

void Chip8Engine_JumpHandler::clearFilledFlagByC8PC(uint16_t c8_pc)
{
	for (int32_t i = 0; i < (int32_t)jump_list.size(); i++) {
		if (jump_list[i]->c8_address_to == c8_pc) {
			jump_list[i]->filled_flag = 0;
		}
	}
}

JUMP_ENTRY * Chip8Engine_JumpHandler::getJumpEntryByIndex(uint32_t index)
{
	return jump_list[index];
}

void Chip8Engine_JumpHandler::DEBUG_printJumpList()
{
	for (int32_t i = 0; i < jump_list.size(); i++) {
		printf("JumpHandler: Jump[%d] c8_address_to = 0x%.4X, x86_address_to = 0x%.8X\n",
			i, jump_list[i]->c8_address_to, (uint32_t)jump_list[i]->x86_address_to);
	}
}

void Chip8Engine_JumpHandler::DEBUG_printCondJumpList()
{
	for (int32_t i = 0; i < cond_jump_list.size(); i++) {
		printf("JumpHandler: CondJump[%d]: c8_address_from = 0x%.4X, c8_address_to = 0x%.4X, x86_address_jump_value = 0x%.8X, ",
			i, cond_jump_list[i]->c8_address_from, cond_jump_list[i]->c8_address_to, (uint32_t)cond_jump_list[i]->x86_address_jump_value);
		printf("translator_cycles = %d\n",
			cond_jump_list[i]->translator_cycles);
	}
}