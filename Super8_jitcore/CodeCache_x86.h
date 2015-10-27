#pragma once

#include <cstdint>
#include <Windows.h>

#define DEFAULT_CACHE_SZ 4096

class CodeCache_x86 {
public:
	uint8_t * cache_mem;
	uint32_t cache_pc;
	uint32_t cache_sz;

	CodeCache_x86();
	~CodeCache_x86();

	void allocCache(uint32_t sz);
	void clearCache();
	void incrementCachePC(uint8_t count);
	bool isCacheEmptyPostSetup();
	void execCache();

	void write8(uint8_t byte_);
	void write16(uint16_t word_);
	void write32(uint32_t dword_);
private:
};