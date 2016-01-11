#pragma once

#include <cstdint>
#include <string>
#include <Windows.h>

#include "Headers\Globals.h"
#include "Headers\FastArrayList\FastArrayList.h"

#ifdef USE_DEBUG_EXTRA
#define  MAX_CACHE_SZ 0xFFFF
#else
#define  MAX_CACHE_SZ 0xFFF
#endif

struct CACHE_REGION {
	uint16_t c8_start_recompile_pc; // The start C8 pc for this cache (code inclusive).
	uint16_t c8_end_recompile_pc; // The end C8 pc for this cache (code inclusive).
	uint8_t c8_pc_alignement; // TESTING: used to signify the byte alignment of this cache. For chip8 roms, this is always either 0 (pc % 2 == 0) or 1 (pc % 2 == 1) as each opcode is always 2 bytes long.
	uint8_t * x86_mem_address; // Used to store the base address of where the cache is stored in memory.
	uint32_t x86_pc; // Used to tell how much code has been emitted to the cache (code size).
	uint8_t stop_write_flag; // Used to signify that no more code should be emitted to this cache (usually because it ends in a jump).
};

class Chip8Engine_CacheHandler : ILogComponent
{
public:
	FastArrayList<int32_t> * cache_invalidate_list;
	int32_t selected_cache_index = 0;
	FastArrayList<CACHE_REGION> * cache_list;

	uint8_t * setup_cache_cdecl;
	uint8_t setup_cache_cdecl_sz;
	uint8_t * setup_cache_return_jmp_address;
	uint8_t * setup_cache_eip_hack;

	Chip8Engine_CacheHandler();
	~Chip8Engine_CacheHandler();

	std::string getComponentName();

	void setupCache_CDECL();
	void execCache_CDECL();
	void initFirstCache();

	// BELOW FUNCTIONS HANDLE ALLOCATION
	int32_t getCacheWritableByStartC8PC(uint16_t c8_jump_pc); // Used when a jump is made (see dynarec.cpp and jumphandler.cpp). Contains more code path logic and handles invalidation.
	int32_t getCacheWritableByC8PC(uint16_t c8_pc);

	void invalidateCacheByFlag();
	void setInvalidFlagByIndex(int32_t index);
	void setInvalidFlagByC8PC(uint16_t c8_pc_);
	uint8_t getInvalidFlagByIndex(int32_t index);

	void setStopWriteFlagCurrent();
	void setStopWriteFlagByIndex(int32_t index);
	void clearStopWriteFlagByIndex(int32_t index);
	uint8_t getStopWriteFlagByIndex(int32_t index);

	// BELOW FUNCTIONS DO NOT ALLOCATE CACHES, THESE ARE ONLY USED FOR FINDING
	int32_t findCacheIndexCurrent();
	int32_t findCacheIndexByC8PC(uint16_t c8_pc_);
	int32_t findCacheIndexByStartC8PC(uint16_t c8_pc_);
	int32_t findCacheIndexByX86Address(uint8_t * x86_address);

	void switchCacheByC8PC(uint16_t c8_pc_);
	void switchCacheByIndex(int32_t index);

	void incrementCacheX86PC(uint8_t count);
	void setCacheEndC8PCCurrent(uint16_t c8_end_pc_);
	void setCacheEndC8PCByIndex(int32_t index, uint16_t c8_end_pc_);
	uint16_t getEndC8PCCurrent();
	uint8_t * getEndX86AddressCurrent();

	CACHE_REGION * getCacheInfoCurrent();
	CACHE_REGION * getCacheInfoByIndex(int32_t index);
	CACHE_REGION * getCacheInfoByC8PC(uint16_t c8_pc_);

	void write8(uint8_t byte_);
	void write16(uint16_t word_);
	void write32(uint32_t dword_);

#ifdef USE_DEBUG_EXTRA
	void DEBUG_printCacheByIndex(int32_t index);
	void DEBUG_printCacheList();
#endif

private:
	int32_t allocNewCacheByC8PC(uint16_t c8_start_pc_); // The main allocation function
	int32_t allocAndSwitchNewCacheByC8PC(uint16_t c8_start_pc_);
	void deallocAllCacheExit();
};
