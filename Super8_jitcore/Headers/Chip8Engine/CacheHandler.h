#pragma once

#include <cstdint>
#include <string>

#include "Headers\Globals.h"

// This define sets the memory size that each cache will be allocated. Currently from my own testing 4096 (0xFFF) bytes seems to be plenty for any rom block thats translated. When the extra debugging flag is defined, the cache size needs to be increased to account for the large number of debug interrupts emitted.
#ifdef USE_DEBUG_EXTRA
#define  MAX_CACHE_SZ 0xFFFF
#else
#define  MAX_CACHE_SZ 0xFFF
#endif

namespace Chip8Engine {

	struct CACHE_REGION {
		uint16_t c8_start_recompile_pc; // The start C8 pc for this cache (code inclusive).
		uint16_t c8_end_recompile_pc; // The end C8 pc for this cache (code inclusive).
		uint8_t * x86_mem_address; // Used to store the base address of where the cache is stored in memory.
		uint32_t x86_pc; // Used to tell how much code has been emitted to the cache (code size).
		uint8_t invalid_flag; // Used in order to support self-modifying code. When this flag is set, it means the cache should be deleted (see PREPARE_FOR_JUMP etc interrupt handlers).
	};

	class CacheHandler : ILogComponent
	{
	public:
		// Cache list variables.
		int32_t selected_cache_index; // This index controls which cache the CacheHandler is accessing within the cache_list vector.
		std::vector<CACHE_REGION> * cache_list; // This vector holds all of the cache info in a list style. When a cache is created its info is stored in here, and removed when the cache has been invalidated.

		uint8_t * setup_cache_cdecl; // This is the entry point address to the CDECL cache, which is the first address that is run when executing the translated caches. See execCache_CDECL().
		uint8_t * setup_cache_cdecl_return_address; // This is the return point address where interrupts jump to in order to get handled.
		uint8_t * setup_cache_cdecl_eip_hack; // This is the address where the EIP x86-32 'hack' resides. See CodeEmitter_x86::DYNAREC_MOV_EAX_EIP().

		CacheHandler();
		~CacheHandler();

		std::string getComponentName();

		void setupCache_CDECL(); // Function creates the CDECL cache, which is used before the recompiled code is run.
		void execCache_CDECL(); // Function starts the execution process of the recompiled caches. This function jumps to the setup_cache_cdecl address variable, which in turn jumps to the caches (X86_STATE::x86_resume_emulation).
		void initFirstCache(); // Function creates the first cache for C8 PC 0x200 and points the X86_STATE::x86_resume_emulation variable to this created cache.

		// CACHE ALLOCATION FUNCTIONS.
		int32_t getCacheWritableByStartC8PC(uint16_t c8_jump_pc); // Used when a jump is made. Either returns the index of a newly created cache, or of an existing cache. Usually when this function is called a new cache will be created.

		// CACHE INVALIDATION FUNCTIONS
		void invalidateCacheByFlag(); // Function deletes caches that are marked invalid here.
		// Functions below either set or get the invalid flag.
		void setInvalidFlagByIndex(int32_t index);
		void setInvalidFlagByC8PC(uint16_t c8_pc_);
		uint8_t getInvalidFlagByIndex(int32_t index);

		// CACHE FINDING FUNCTIONS (no allocation occurs from these).
		int32_t findCacheIndexCurrent();
		int32_t findCacheIndexByC8PC(uint16_t c8_pc_);
		int32_t findCacheIndexByStartC8PC(uint16_t c8_pc_);
		int32_t findCacheIndexByX86Address(uint8_t * x86_address);

		// CACHE SWITCHING FUNCTIONS (change selected_cache_index).
		void switchCacheByC8PC(uint16_t c8_pc_);
		void switchCacheByIndex(int32_t index);

		// Get cache info functions.
		CACHE_REGION * getCacheInfoCurrent();
		CACHE_REGION * getCacheInfoByIndex(int32_t index);

		// Cache parameter functions.
		void incrementCacheX86PC(uint8_t count);
		void setCacheEndC8PCCurrent(uint16_t c8_end_pc_);
		void setCacheEndC8PCByIndex(int32_t index, uint16_t c8_end_pc_);
		uint16_t getEndC8PCCurrent();
		uint8_t * getEndX86AddressCurrent();

		// Raw cache writing functions. These functions are used by the CodeEmitter_x86 class (mostly), for emitting the raw x86 bytes that need to be executed.
		void write8(uint8_t byte_);
		void write16(uint16_t word_);
		void write32(uint32_t dword_);

#ifdef USE_DEBUG_EXTRA
		void DEBUG_printCacheByIndex(int32_t index);
		void DEBUG_printCacheList();
#endif

	private:
		// Allocation and deallocation functions, but are never used by other components. Instead there are other functions within this class that call these.
		int32_t allocNewCacheByC8PC(uint16_t c8_start_pc_); // The main allocation function. Allocates a new cache block in memory and adds it to the cache_list (returning the index).
		int32_t allocAndSwitchNewCacheByC8PC(uint16_t c8_start_pc_); // Same as above, but also changes selected_cache_index.
		void deallocAllCacheExit(); // Used upon exit to deallocate and free memory.
	};

}