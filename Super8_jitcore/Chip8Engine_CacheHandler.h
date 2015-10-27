#pragma once

#include <cstdint>
#include <vector>
#include <Windows.h>

#include "Chip8Globals.h"

#define  MAX_CACHE_SZ 0xFFF

struct MEM_REGION {
	uint16_t c8_start_recompile_pc;
	uint16_t c8_end_recompile_pc;
	uint8_t * x86_mem_address;
	uint32_t x86_pc;
	uint8_t invalid_flag; // used with indirect jumps
	uint8_t stop_write_flag; // used to signify that no more code should be emitted to this cache (usually because it ends in a jump)
};

class Chip8Engine_CacheHandler
{
public:
	int32_t selected_cache_index;
	std::vector<MEM_REGION> memory_maps;

	uint8_t * setup_cache_cdecl;
	uint8_t setup_cache_cdecl_sz;
	uint8_t * setup_cache_return_jmp_address;
	uint8_t * setup_cache_eip_hack;

	Chip8Engine_CacheHandler();
	~Chip8Engine_CacheHandler();

	void setupCache_CDECL();
	void execCache_CDECL();

	int32_t allocNewMemoryRegion();
	int32_t allocAndSwitchNewMemoryRegion();
	int32_t checkMemoryRegionAllocatedByC8PC(uint16_t c8_pc_);
	int32_t checkAndSwitchMemoryRegionAllocatedByC8PC(uint16_t c8_pc_);
	int32_t allocNewMemoryRegionByC8PC(uint16_t c8_start_pc_);
	int32_t allocAndSwitchNewMemoryRegionByC8PC(uint16_t c8_start_pc_);

	void invalidateMemoryRegion();
	void invalidateAllMemoryRegionsByFlag();
	void invalidateMemoryRegionByC8PC(uint16_t c8_start_pc_);
	void invalidateMemoryRegionByIndex(int32_t index);

	void setInvalidFlag(uint8_t value);
	void setInvalidFlagByIndex(int32_t index, uint8_t value);
	uint8_t getInvalidFlagByIndex(int32_t index);

	void stopWriteMemoryRegion();
	uint8_t getStopWriteMemoryRegionByIndex(int32_t index);

	int32_t getMemoryRegionIndex();
	void switchMemoryRegionByC8PC(uint16_t c8_pc_);
	void switchMemoryRegionByIndex(uint32_t index);

	void incrementCacheC8Region(uint16_t c8_pc_offset_);
	void setMemoryRegionC8EndPC(uint16_t c8_end_pc_);
	void setMemoryRegionC8EndPCByIndex(uint32_t index, uint16_t c8_end_pc_);

	MEM_REGION * getMemoryRegionInfo();
	MEM_REGION * getMemoryRegionInfoByIndex(uint32_t index);
	MEM_REGION * getMemoryRegionInfoByC8PC(uint16_t c8_pc_);
	uint16_t getMemoryRegionC8EndPC();
	uint8_t * getMemoryRegionCurrentx86Address();
	int32_t getMemoryRegionIndexByX86Address(uint8_t * x86_address);

	void clearCache();
	void write8(uint8_t byte_);
	void write16(uint16_t word_);
	void write32(uint32_t dword_);
	void incrementCacheX86PC(uint8_t count);
};

