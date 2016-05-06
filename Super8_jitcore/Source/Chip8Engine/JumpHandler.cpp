#include "stdafx.h"

#include <cstdint>
#include <vector>

#include "Headers\Globals.h"

#include "Headers\Chip8Globals\MainEngineGlobals.h"
#include "Headers\Chip8Engine\JumpHandler.h"
#include "Headers\Chip8Engine\CacheHandler.h"

using namespace Chip8Globals::MainEngineGlobals;
using namespace Chip8Engine;

JumpHandler::JumpHandler()
{
	jump_list = new std::vector<JUMP_ENTRY>();
	(*jump_list).reserve(1024);
	cond_jump_list = new std::vector<COND_JUMP_ENTRY>();
	(*cond_jump_list).reserve(1024);

	// Register this component in logger
	logger->registerComponent(this);
}

JumpHandler::~JumpHandler()
{
	// Deregister this component in logger
	logger->deregisterComponent(this);

	delete cond_jump_list;
	delete jump_list;
}

std::string JumpHandler::getComponentName()
{
	return std::string("JumpHandler");
}

int32_t JumpHandler::recordJumpEntry(uint16_t c8_to_)
{
	JUMP_ENTRY entry;
	entry.c8_address_to = c8_to_;
	entry.x86_address_to = NULL; // JMP_M_PTR_32 uses this value (unknown at beginning)
	entry.filled_flag = 0;
	(*jump_list).push_back(entry);
#ifdef USE_VERBOSE
	char buffer[1000];
	sprintf_s(buffer, 1000, "Jump[%d] recorded. C8_to = 0x%.4X.", jump_list->size() - 1, c8_to_);
	logMessage(LOGLEVEL::L_INFO, buffer);
#endif
	return ((*jump_list).size() - 1);
}

int32_t JumpHandler::recordConditionalJumpEntry(uint16_t c8_from_, uint16_t c8_to_, uint8_t translator_cycles_, uint32_t * x86_address_jump_value_)
{
	COND_JUMP_ENTRY entry;
	entry.c8_address_from = c8_from_;
	entry.c8_address_to = c8_to_;
	entry.x86_address_jump_value = x86_address_jump_value_;
	entry.translator_cycles = translator_cycles_;
	(*cond_jump_list).push_back(entry);
#ifdef USE_VERBOSE
	char buffer[1000];
	sprintf_s(buffer, 1000, "Conditional Jump[%d] recorded. C8_from = 0x%.4X, C8_to = 0x%.4X, x86_address = 0x%.8X, cycles = %d.", cond_jump_list->size() - 1, c8_from_, c8_to_, (uint32_t)x86_address_jump_value_, translator_cycles_);
	logMessage(LOGLEVEL::L_INFO, buffer);
#endif
	return ((*cond_jump_list).size() - 1);
}

void JumpHandler::decreaseConditionalCycle()
{
	for (int32_t i = 0; i < (int32_t)(*cond_jump_list).size(); i++) {
		if ((*cond_jump_list)[i].translator_cycles > 0) {
			(*cond_jump_list)[i].translator_cycles -= 1;
		}
	}
}

void JumpHandler::checkAndFillConditionalJumpsByCycles()
{
	for (int32_t i = 0; i < (int32_t)(*cond_jump_list).size(); i++) {
		if ((*cond_jump_list)[i].translator_cycles == 0) {
			// If a conditional jump has cycles = 0, then use the current cache address (getEndX86AddressCurrent()) as the position to jump to, relative from the jump value address (x86_address_jump_value).
			int32_t relative = (int32_t)((uint32_t)cachehandler->getEndX86AddressCurrent() - (uint32_t)(*cond_jump_list)[i].x86_address_jump_value - sizeof(uint32_t)); // 4 is size of uint32_t, as eip is at the end of the jump instruction but we calculate the relative size based on the start address of the relative
			*((*cond_jump_list)[i].x86_address_jump_value) = relative;
#ifdef USE_VERBOSE
			char buffer[1000];
			sprintf_s(buffer, 1000, "Conditional Jump[%d] updated! Value %d written to 0x%.8X (in cache[%d]).", i, relative, (uint32_t)(*cond_jump_list)[i].x86_address_jump_value, cache->findCacheIndexCurrent());
			logMessage(LOGLEVEL::L_INFO, buffer);
#endif

			// remove entry after its been filled
			(*cond_jump_list).erase((*cond_jump_list).begin() + i);
			i -= 1; // decrease i by 1 so it rechecks the current i'th value in the list (which would have been i+1 if there was no remove).
		}
	}
}

int32_t JumpHandler::findJumpEntry(uint16_t c8_to_)
{
	int32_t index = -1;
	for (int32_t i = 0; i < (int32_t)(*jump_list).size(); i++) {
		if (c8_to_ == (*jump_list)[i].c8_address_to) {
			index = i;
			break;
		}
	}
	return index;
}

void JumpHandler::checkAndFillJumpsByStartC8PC()
{
	// Function makes sure that there is a valid jump location for ANY recorded jump table entry, as determined by the filled_flag. If there is no currently allocated cache for the Chip8 jump location, it will create a new cache through the CacheHandler and record the starting x86 memory address in the jump table entry.
	int32_t cache_index;
	for (int32_t i = 0; i < (int32_t)(*jump_list).size(); i++) {
		if ((*jump_list)[i].filled_flag == 0) {
			// Jump cache handling done by CacheHandler, so this function just updates the jump table locations
			cache_index = cachehandler->getCacheWritableByStartC8PC((*jump_list)[i].c8_address_to);
			(*jump_list)[i].x86_address_to = cachehandler->getCacheInfoByIndex(cache_index)->x86_mem_address;

			// Set filled flag to 1 after it has been handled
			(*jump_list)[i].filled_flag = 1;
		}
	}
}

void JumpHandler::clearFilledFlagByC8PC(uint16_t c8_pc)
{
	for (int32_t i = 0; i < (int32_t)(*jump_list).size(); i++) {
		if ((*jump_list)[i].c8_address_to == c8_pc) {
			(*jump_list)[i].filled_flag = 0;
		}
	}
}

int32_t JumpHandler::getJumpIndexByC8PC(uint16_t c8_to)
{
	int32_t tblindex = jumphandler->findJumpEntry(c8_to);
	if (tblindex == -1) {
		tblindex = jumphandler->recordJumpEntry(c8_to);
	}
	return tblindex;
}

JUMP_ENTRY * JumpHandler::findJumpInfoByIndex(uint32_t index)
{
	return &(*jump_list)[index];
}

#ifdef USE_DEBUG_EXTRA
void Chip8Engine_JumpHandler::DEBUG_printJumpList()
{
	for (int32_t i = 0; i < (int32_t)(*jump_list).size(); i++) {
		char buffer[1000];
		sprintf_s(buffer, 1000, "Jump[%d] c8_address_to = 0x%.4X, x86_address_to = 0x%.8X.", i, (*jump_list)[i].c8_address_to, (uint32_t)(*jump_list)[i].x86_address_to);
		logMessage(LOGLEVEL::L_DEBUG, buffer);
	}
}

void Chip8Engine_JumpHandler::DEBUG_printCondJumpList()
{
	for (int32_t i = 0; i < (int32_t)(*cond_jump_list).size(); i++) {
		char buffer[1000];
		sprintf_s(buffer, 1000, "CondJump[%d]: c8_address_from = 0x%.4X, c8_address_to = 0x%.4X, x86_address_jump_value = 0x%.8X, translator_cycles = %d.", i, (*cond_jump_list)[i].c8_address_from, (*cond_jump_list)[i].c8_address_to, (uint32_t)(*cond_jump_list)[i].x86_address_jump_value, (*cond_jump_list)[i].translator_cycles);
		logMessage(LOGLEVEL::L_DEBUG, buffer);
	}
}
#endif