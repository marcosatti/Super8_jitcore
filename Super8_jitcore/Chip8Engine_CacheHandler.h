#pragma once

#include <cstdint>
#include <vector>
#include <Windows.h>

#include "Chip8Globals.h"

#define  MAX_CACHE_SZ 0xFFF

struct CACHE_REGION {
	uint16_t c8_start_recompile_pc;
	uint16_t c8_end_recompile_pc;
	uint8_t * x86_mem_address;
	uint32_t x86_pc;
	uint8_t invalid_flag; // used with jumps. 1 means marked for deletion after run once, 2 means marked for deletion asap.
	uint8_t stop_write_flag; // used to signify that no more code should be emitted to this cache (usually because it ends in a jump)
};

class Chip8Engine_CacheHandler
{
public:
	int32_t selected_cache_index;
	std::vector<CACHE_REGION> cache_list;

	uint8_t * setup_cache_cdecl;
	uint8_t setup_cache_cdecl_sz;
	uint8_t * setup_cache_return_jmp_address;
	uint8_t * setup_cache_eip_hack;

	Chip8Engine_CacheHandler();
	~Chip8Engine_CacheHandler();

	void setupCache_CDECL();
	void execCache_CDECL();

	int32_t allocNewCacheByC8PC(uint16_t c8_start_pc_); // The main allocation function
	int32_t allocNewCacheByJumpC8PC(uint16_t c8_jump_pc); // Used when a jump is made (see dynarec.cpp and jumphandler.cpp). Contains more code path logic and handles invalidation.
	int32_t allocAndSwitchNewCacheByC8PC(uint16_t c8_start_pc_);

	void invalidateCacheCurrent();
	void invalidateCacheByFlag();
	void invalidateCacheByIndex(int32_t index);
	void invalidateCacheByC8PC(uint16_t c8_start_pc_);

	void setInvalidFlagCurrent();
	uint8_t setInvalidFlagByIndex(int32_t index);
	uint8_t setInvalidFlagByC8PC(uint16_t c8_pc_);
	void clearInvalidFlagByIndex(int32_t index);
	uint8_t getInvalidFlagByIndex(int32_t index);

	void setStopWriteFlagCurrent(uint8_t value);
	void setStopWriteFlagByIndex(int32_t index, uint8_t value);
	uint8_t getStopWriteFlagByIndex(int32_t index);
	uint8_t getStopWriteFlagByC8PC(uint16_t c8_pc_);

	int32_t getCacheIndexCurrent();
	int32_t getCacheIndexByC8PC(uint16_t c8_pc_);
	int32_t getCacheIndexByStartC8PC(uint16_t c8_pc_);
	int32_t getCacheIndexByX86Address(uint8_t * x86_address);
	void switchCacheByC8PC(uint16_t c8_pc_);
	void switchCacheByIndex(uint32_t index);

	void setCacheEndC8PCCurrent(uint16_t c8_end_pc_);
	void setCacheEndC8PCByIndex(uint32_t index, uint16_t c8_end_pc_);
	uint16_t getEndC8PCCurrent();
	uint8_t * getEndX86AddressCurrent();

	CACHE_REGION * getCacheInfoCurrent();
	CACHE_REGION * getCacheInfoByIndex(uint32_t index);
	CACHE_REGION * getCacheInfoByC8PC(uint16_t c8_pc_);

	void write8(uint8_t byte_);
	void write16(uint16_t word_);
	void write32(uint32_t dword_);
	void incrementCacheX86PC(uint8_t count);

	void formatCache(); // Sets cache contents to 0x90 NOP

	void DEBUG_printCacheByIndex(int32_t index);
	void DEBUG_printCacheList();
};

