#include "stdafx.h"
#include "CodeCache_x86.h"

CodeCache_x86::CodeCache_x86() {

}

CodeCache_x86::~CodeCache_x86() {

}

void CodeCache_x86::allocCache(uint32_t sz)
{
	cache_sz = sz;

	// WIN32 specific
	cache_mem = (uint8_t *)VirtualAlloc(0, cache_sz, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	
	// DEBUG
	printf("Cache allocated. Location and size: %p, %d\n", cache_mem, cache_sz);

	memset(cache_mem, 0x90, cache_sz);
	cache_pc = 0;
}

void CodeCache_x86::clearCache()
{
	memset(cache_mem, 0x90, cache_sz);
	cache_pc = 0;
	
	// DEBUG
	//printf("\nCache cleared and cache_pc reset to 0.\n");
}

void CodeCache_x86::incrementCachePC(uint8_t count)
{
	cache_pc += count;
}

bool CodeCache_x86::isCacheEmptyPostSetup()
{
	if (cache_pc == 3) return true; // cache_pc = 3 when there is only the setup pop/push code in there (see translater loop)
	return false;
}

void CodeCache_x86::execCache()
{
	// DEBUG
	//printf("\n!!!Printing cache to screen!!!\n");
	//for (int i = 0; i < cache_pc; i++) {
	//	printf("0x%.2X, ", *(cache_mem + i));
	//}
	//printf("\n");

	void(*exec)() = (void(*)())cache_mem;
	exec();
}

void CodeCache_x86::write8(uint8_t byte_)
{
	*(cache_mem + cache_pc) = byte_;
	// DEBUG
	//printf("Cache byte written. location and value: %p + %X, %.2X\n", cache_mem, cache_pc, byte_);
	incrementCachePC(1);
}

void CodeCache_x86::write16(uint16_t word_)
{
	uint8_t* cache_mem_current = cache_mem + cache_pc;
	*((uint16_t*)cache_mem_current) = word_;
	// DEBUG
	//printf("Cache word written. location and value: %p + %X, %.4X\n", cache_mem, cache_pc, word_);
	incrementCachePC(2);
}

void CodeCache_x86::write32(uint32_t dword_)
{
	uint8_t* cache_mem_current = cache_mem + cache_pc;
	*((uint32_t*)cache_mem_current) = dword_;
	// DEBUG
	//printf("Cache dword written. location and value: %p + %X, %.8X\n", cache_mem, cache_pc, dword_);
	incrementCachePC(4);
}
