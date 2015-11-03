#include "stdafx.h"
#include "Chip8Engine_CacheHandler.h"

using namespace Chip8Globals;

Chip8Engine_CacheHandler::Chip8Engine_CacheHandler()
{
	setup_cache_cdecl = NULL;
	cache_list.reserve(1024);
}

Chip8Engine_CacheHandler::~Chip8Engine_CacheHandler()
{
}

void Chip8Engine_CacheHandler::setupCache_CDECL()
{
	// A small cache which is used to handle the CDECL call convention before passing off to the main cache execution point
	// WIN32 specific
	if (setup_cache_cdecl == NULL) {
		// Alloc cdecl setup cache for first time. will not change after this
		uint8_t	bytes[] = {
			0x55,					//0x0 PUSH pop
			0x89,					//0x1 (1) MOV ebp, esp
			0b11101100,				//0x2 (2, MODRM) MOV ebp, esp
			0xFF,					//0x3 (1) JMP r/m32
			0b00100101,				//0x4 (2, MODRM) JMP r/m32
			0x00,					//0x5 (3, DISP32)
			0x00,					//0x6 (4, DISP32)
			0x00,					//0x7 (5, DISP32)
			0x00,					//0x8 (6, DISP32)
			0x5D,					//0x9 POP ebp
			0xC3,					//0xA RET

									// HACK: ASM BELOW USED TO GET EIP ADDRESS AND RETURN IN EAX. SEE CodeEmitter_x86->DYNAREC_MOV_EAX_EIP
			0x58,					//0xB POP eax
			0x50,					//0xC PUSH eax
			0xC3					//0xD RET
		};

		setup_cache_cdecl_sz = sizeof(bytes) / sizeof(bytes[0]);

		setup_cache_cdecl = (uint8_t *)VirtualAlloc(0, setup_cache_cdecl_sz, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		memcpy(setup_cache_cdecl, bytes, setup_cache_cdecl_sz);
		setup_cache_return_jmp_address = (setup_cache_cdecl + 0x9);
		setup_cache_eip_hack = (setup_cache_cdecl + 0xB);

		// DEBUG
		printf("Cache: CDECL Cache allocated. Location and size: 0x%.8X, %d\n", (uint32_t)setup_cache_cdecl, setup_cache_cdecl_sz);
		printf("       setup_cache_return_jmp_address @ location 0x%.8X\n", (uint32_t)&setup_cache_return_jmp_address);
		printf("       setup_cache_eip_hack @ location 0x%.8X\n", (uint32_t)&setup_cache_eip_hack);
		printf("       x86_resume_address @ location 0x%.8X\n", (uint32_t)&X86_STATE::x86_resume_address);
	}

	// Update cdecl cache with location of x86_resume_address variable
	*(uint32_t *)(setup_cache_cdecl + 0x5) = (uint32_t)&X86_STATE::x86_resume_address;

	printf("Cache: updated CDECL Cache. Bytes: ");
	for (int i = 0; i < setup_cache_cdecl_sz; i++) {
		printf("0x%.2X,", *(setup_cache_cdecl + i));
	}
	printf("\n");
}

void Chip8Engine_CacheHandler::execCache_CDECL()
{
	// DEBUG
	//printf("\n!!!Printing cache to screen!!!\n");
	//for (int i = 0; i < cache_pc; i++) {
	//	printf("0x%.2X, ", *(cache_mem + i));
	//}
	//printf("\n");
	//printf("\nCache: Attempting to execute cdecl cache. x86_resume_address = 0x%.8X (cache[%d])\n", (uint32_t)X86_STATE::x86_resume_address, cache->getCacheIndexByX86Address(X86_STATE::x86_resume_address));
	void(*exec)() = (void(*)())setup_cache_cdecl;
	exec();
}

void Chip8Engine_CacheHandler::initFirstCache()
{
	allocAndSwitchNewCacheByC8PC(C8_STATE::cpu.pc);
	X86_STATE::x86_resume_address = getCacheInfoCurrent()->x86_mem_address;
}

int32_t Chip8Engine_CacheHandler::getCacheIndexByC8PC(uint16_t c8_pc_)
{
	int32_t index = -1;
	for (uint32_t i = 0; i < cache_list.size(); i++) {
		if (c8_pc_ >= cache_list[i].c8_start_recompile_pc && c8_pc_ <= cache_list[i].c8_end_recompile_pc && cache_list[i].invalid_flag == 0) {
			index = i;
			break;
		}
	}
	return index;
}

int32_t Chip8Engine_CacheHandler::getCacheIndexByStartC8PC(uint16_t c8_pc_)
{
	int32_t index = -1;
	for (uint32_t i = 0; i < cache_list.size(); i++) {
		if (c8_pc_ == cache_list[i].c8_start_recompile_pc && cache_list[i].invalid_flag == 0) {
			index = i;
			break;
		}
	}
	return index;
}

int32_t Chip8Engine_CacheHandler::allocNewCacheByC8PC(uint16_t c8_start_pc_)
{
	// WIN32 specific
	uint8_t * cache_mem = (uint8_t *)VirtualAlloc(0, MAX_CACHE_SZ, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	// set to NOP 0x90
	memset(cache_mem, 0x90, MAX_CACHE_SZ);

	// set last memory bytes to OUT_OF_CODE interrupt
	// Emits change x86_status_code to 2 (out of code) & x86_resume_c8_pc = c8_start_pc then jump back to cdecl return address
	uint8_t bytes[] = {
		0xC6, // (0) MOV m, Imm8
		0b00000101, // (1) MOV m, Imm8
		0x00, // (2) PTR 32 
		0x00, // (3) PTR 32
		0x00, // (4) PTR 32
		0x00, // (5) PTR 32
		0x02, // (6) X86_STATUS_CODE = 2 (OUT_OF_CODE)
		0x66, // (7) 66h prefix (16bit)
		0xC7, // (8) MOV m, Imm16
		0b00000101, // (9) MOV m, Imm16
		0x00, // (10) PTR 32 
		0x00, // (11) PTR 32
		0x00, // (12) PTR 32
		0x00, // (13) PTR 32
		0x00, // (14) X86_RESUME_C8_PC
		0x00, // (15) X86_RESUME_C8_PC
		0xFF, // (16) JMP PTR 32
		0b00100101, // (17) JMP PTR 32
		0x00, // (18) PTR 32
		0x00, // (19) PTR 32
		0x00, // (20) PTR 32
		0x00 // (21) PTR 32
	};
	uint32_t x86_status_code_address = (uint32_t)&(X86_STATE::x86_status_code);
	uint32_t x86_c8_pc_address = (uint32_t)&(X86_STATE::x86_resume_c8_pc);
	uint32_t cdecl_return_address = (uint32_t)&cache->setup_cache_return_jmp_address;
	*((uint32_t*)(bytes + 2)) = x86_status_code_address;
	*((uint32_t*)(bytes + 10)) = x86_c8_pc_address;
	*((uint16_t*)(bytes + 14)) = c8_start_pc_;
	*((uint32_t*)(bytes + 18)) = cdecl_return_address;
	uint8_t sz = sizeof(bytes) / sizeof(bytes[0]);
	memcpy(cache_mem + MAX_CACHE_SZ - sz, bytes, sz); // Write this to last bytes of cache

	// cache end pc is unknown at allocation, so set to start pc too (it is known if its a new cache by checking start==end pc)
	CACHE_REGION memoryblock = { c8_start_pc_ ,c8_start_pc_ , cache_mem, 0, 0, 0 };
	cache_list.push_back(memoryblock);

	// DEBUG
	printf("CacheHandler: Cache[%d] allocated. Location and size: %p, %d, C8 Start PC = 0x%.4X\n", cache_list.size() - 1, cache_mem, MAX_CACHE_SZ, c8_start_pc_);

	return (cache_list.size() - 1);
}

int32_t Chip8Engine_CacheHandler::getCacheByStartC8PC(uint16_t c8_jump_pc)
{
	int32_t index = cache->getCacheIndexByStartC8PC(c8_jump_pc);
	if (index == -1) {
		// No cache was found, so check if theres a cache with the pc in the middle 
		index = getCacheIndexByC8PC(c8_jump_pc);
		if (index == -1) {
			// No cache was found at all, so allocate a completely new cache
			index = allocNewCacheByC8PC(c8_jump_pc);
			//printf("CacheHandler: Jump Cache Path Result = NEW CACHE(%d)\n", index);
		}
		else {
			// A cache was found where the jump pc is located in the middle of the cache.
			// Need to mark invalid (either 1 or 2) and alloc a new cache, as this code path for the cache wont be run again (next time the new cache will be found)
			uint8_t result = cache->setInvalidFlagByIndex(index);
			int32_t old_index = index;
			index = allocNewCacheByC8PC(c8_jump_pc);
			//printf("CacheHandler: Jump Cache Path Result = INVALIDATE(%d,%d) & NEW CACHE(%d)\n", old_index, result, index);
		}

	}
	else {
		// dont need to allocate/invalidate anything here, as jump will be to the start C8 PC requested
		//printf("CacheHandler: Jump Cache Path Result = FOUND CACHE(%d)\n", index);
	}
	return index;
}

int32_t Chip8Engine_CacheHandler::allocAndSwitchNewCacheByC8PC(uint16_t c8_start_pc_)
{
	uint32_t index = allocNewCacheByC8PC(c8_start_pc_);
	switchCacheByIndex(index);
	return index;
}

void Chip8Engine_CacheHandler::invalidateCacheCurrent()
{
	printf("CacheHandler: Cache[%d] invalidated. C8 Start PC = 0x%.4X, C8 End PC = 0x%.4X\n", selected_cache_index, cache_list[selected_cache_index].c8_start_recompile_pc, cache_list[selected_cache_index].c8_end_recompile_pc);
	// Invalidates (deletes) the current memory region selected.
	VirtualFree(cache_list[selected_cache_index].x86_mem_address, 0, MEM_RELEASE);
	cache_list.erase(cache_list.begin() + selected_cache_index);

	// Handle selected_cache_index changes (set to -1, will need to reselect later)
	selected_cache_index = -1;
	printf("               New selected_cache_index = %d\n", selected_cache_index);
}

void Chip8Engine_CacheHandler::invalidateCacheByFlag()
{
	for (int32_t i = 0; i < (int32_t)cache_list.size(); i++) {
		if (cache_list[i].invalid_flag == 2) {
			// Delete cache here
			printf("CacheHandler: Cache[%d] invalidated. C8 Start PC = 0x%.4X, C8 End PC = 0x%.4X\n", i, cache_list[i].c8_start_recompile_pc, cache_list[i].c8_end_recompile_pc);
			VirtualFree(cache_list[i].x86_mem_address, 0, MEM_RELEASE);
			cache_list.erase(cache_list.begin() + i);

			// Handle selected_cache_index changes
			if (selected_cache_index > i) {
				// Need to decrease selected_cache by 1
				selected_cache_index -= 1;
			}
			//else if (selected_cache_index < i) {
			// Do nothing, we are ok
			//}
			else if (selected_cache_index == i) {
				// Set to -1 (need to reselect later)
				selected_cache_index = -1;
			}
			printf("               New selected_cache_index = %d\n", selected_cache_index);
		}
		else if (cache_list[i].invalid_flag == 1) {
			// Check if cache can be deleted next run by looking at x86_resume_addr and determine if its in the cache
			if (i != cache->getCacheIndexByX86Address(X86_STATE::x86_resume_address)) {
				cache_list[i].invalid_flag = 2;
			}
		}
	}
}

void Chip8Engine_CacheHandler::invalidateCacheByC8PC(uint16_t c8_pc_)
{
	for (int32_t i = 0; i < (int32_t)cache_list.size(); i++) {
		if (c8_pc_ >= cache_list[i].c8_start_recompile_pc && c8_pc_ <= cache_list[i].c8_end_recompile_pc) {
			printf("CacheHandler: Cache[%d] invalidated. C8 Start PC = 0x%.4X, C8 End PC = 0x%.4X\n", i, cache_list[i].c8_start_recompile_pc, cache_list[i].c8_end_recompile_pc);
			VirtualFree(cache_list[i].x86_mem_address, 0, MEM_RELEASE);
			cache_list.erase(cache_list.begin() + i);

			// Handle selected_cache_index changes
			if (selected_cache_index > i) {
				// Need to decrease selected_cache by 1
				selected_cache_index -= 1;
			}
			//else if (selected_cache_index < i) {
				// Do nothing, we are ok
			//}
			else if (selected_cache_index == i) {
				// Set to -1 (need to reselect later)
				selected_cache_index = -1;
			}
			printf("               New selected_cache_index = %d\n", selected_cache_index);
			break;
		}
	}
}

void Chip8Engine_CacheHandler::invalidateCacheByIndex(int32_t index)
{
	printf("CacheHandler: Cache[%d] invalidated. C8 Start PC = 0x%.4X, C8 End PC = 0x%.4X\n", index, cache_list[index].c8_start_recompile_pc, cache_list[index].c8_end_recompile_pc);
	VirtualFree(cache_list[index].x86_mem_address, 0, MEM_RELEASE);
	cache_list.erase(cache_list.begin() + index);

	// Handle selected_cache_index changes
	if (selected_cache_index > index) {
		// Need to decrease selected_cache by 1
		selected_cache_index -= 1;
	}
	//else if (selected_cache_index < index) {
	// Do nothing, we are ok
	//}
	else if (selected_cache_index == index) {
		// Set to -1 (need to reselect later)
		selected_cache_index = -1;
	}
	printf("               New selected_cache_index = %d\n", selected_cache_index);
	
}

void Chip8Engine_CacheHandler::setInvalidFlagCurrent()
{
	setInvalidFlagByIndex(selected_cache_index);
}

uint8_t Chip8Engine_CacheHandler::setInvalidFlagByIndex(int32_t index)
{
	if (index == cache->getCacheIndexByX86Address(X86_STATE::x86_resume_address)) {
		// x86_resume_addr is in same cache, so can mark for deletion after the cache has finished being used
		cache_list[index].invalid_flag = 1;
		return 1;
	}
	else {
		// x86_resume_addr is not in same cache, so can mark for deletion immediately (it will be recreated if needed by prep_for_jump)
		cache_list[index].invalid_flag = 2;
		return 2;
	}
}

uint8_t Chip8Engine_CacheHandler::setInvalidFlagByC8PC(uint16_t c8_pc_)
{
	uint8_t status = 0;
	for (uint32_t i = 0; i < cache_list.size(); i++) {
		if (c8_pc_ >= cache_list[i].c8_start_recompile_pc && c8_pc_ <= cache_list[i].c8_end_recompile_pc && cache_list[i].invalid_flag == 0) {
			status = setInvalidFlagByIndex(i);
			break;
		}
	}
	return status;
}

void Chip8Engine_CacheHandler::clearInvalidFlagByIndex(int32_t index)
{
	cache_list[index].invalid_flag = 0;
}

uint8_t Chip8Engine_CacheHandler::getInvalidFlagByIndex(int32_t index)
{
	return cache_list[index].invalid_flag;
}

void Chip8Engine_CacheHandler::setStopWriteFlagCurrent(uint8_t value)
{
	cache_list[selected_cache_index].stop_write_flag = value;
}

void Chip8Engine_CacheHandler::setStopWriteFlagByIndex(int32_t index, uint8_t value)
{
	cache_list[index].stop_write_flag = value;
}

uint8_t Chip8Engine_CacheHandler::getStopWriteFlagByIndex(int32_t index)
{
	return cache_list[index].stop_write_flag;
}

uint8_t Chip8Engine_CacheHandler::getStopWriteFlagByC8PC(uint16_t c8_pc_)
{
	for (uint32_t i = 0; i < cache_list.size(); i++) {
		if (c8_pc_ >= cache_list[i].c8_start_recompile_pc && c8_pc_ <= cache_list[i].c8_end_recompile_pc && cache_list[i].invalid_flag == 0) {
			return cache_list[i].stop_write_flag;
		}
	}
	return 0xFF;
}

void Chip8Engine_CacheHandler::switchCacheByC8PC(uint16_t c8_pc_)
{
	for (uint32_t i = 0; i < cache_list.size(); i++) {
		if (c8_pc_ >= cache_list[i].c8_start_recompile_pc && c8_pc_ <= cache_list[i].c8_end_recompile_pc && cache_list[i].invalid_flag == 0) {
			selected_cache_index = i;
			// set C8 pc to end of memory region
			C8_STATE::cpu.pc = cache_list[selected_cache_index].c8_end_recompile_pc;
			break;
		}
	}
}

int32_t Chip8Engine_CacheHandler::getCacheIndexCurrent()
{
	return selected_cache_index;
}

void Chip8Engine_CacheHandler::switchCacheByIndex(uint32_t index)
{
	selected_cache_index = index;
}

void Chip8Engine_CacheHandler::setCacheEndC8PCCurrent(uint16_t c8_end_pc_)
{
	if (cache_list[selected_cache_index].c8_start_recompile_pc == 0xFFFF) cache_list[selected_cache_index].c8_start_recompile_pc = c8_end_pc_;
	cache_list[selected_cache_index].c8_end_recompile_pc = c8_end_pc_;
}

void Chip8Engine_CacheHandler::setCacheEndC8PCByIndex(uint32_t index, uint16_t c8_end_pc_)
{
	if (cache_list[index].c8_start_recompile_pc == 0xFFFF) cache_list[index].c8_start_recompile_pc = c8_end_pc_;
	cache_list[index].c8_end_recompile_pc = c8_end_pc_;
}

uint16_t Chip8Engine_CacheHandler::getEndC8PCCurrent()
{
	return cache_list[selected_cache_index].c8_end_recompile_pc;
}

uint8_t * Chip8Engine_CacheHandler::getEndX86AddressCurrent()
{
	uint8_t* cache_mem_current = cache_list[selected_cache_index].x86_mem_address + cache_list[selected_cache_index].x86_pc;
	return cache_mem_current;
}

int32_t Chip8Engine_CacheHandler::getCacheIndexByX86Address(uint8_t * x86_address)
{
	int32_t index = -1;
	for (uint32_t i = 0; i < cache_list.size(); i++) {
		if (x86_address >= cache_list[i].x86_mem_address && x86_address <= (cache_list[i].x86_mem_address + cache_list[i].x86_pc)) {
			index = i;
			break;
		}
	}
	return index;
}

CACHE_REGION * Chip8Engine_CacheHandler::getCacheInfoCurrent()
{
	return &(cache_list[selected_cache_index]);
}

CACHE_REGION * Chip8Engine_CacheHandler::getCacheInfoByIndex(uint32_t index)
{
	return &(cache_list[index]);
}

CACHE_REGION * Chip8Engine_CacheHandler::getCacheInfoByC8PC(uint16_t c8_pc_)
{
	uint32_t idx = -1;
	for (uint32_t i = 0; i < cache_list.size(); i++) {
		if (c8_pc_ >= cache_list[i].c8_start_recompile_pc && c8_pc_ <= cache_list[i].c8_end_recompile_pc) {
			idx = i;
			break;
		}
	}
	return &(cache_list[idx]);
}

void Chip8Engine_CacheHandler::formatCache()
{
	memset(cache_list[selected_cache_index].x86_mem_address, 0x90, MAX_CACHE_SZ);
	cache_list[selected_cache_index].x86_pc = 0;

	// DEBUG
	printf("Cache: Cache[%d] cleared and cache_pc reset to 0.\n", selected_cache_index);
}

void Chip8Engine_CacheHandler::DEBUG_printCacheByIndex(int32_t index)
{
	printf("CacheHandler: Cache[%d]: C8_start_pc = 0x%.4X, C8_end_pc = 0x%.4X, X86_mem_address = 0x%.8X, X86_pc = 0x%.8X\n",
		index, cache_list[index].c8_start_recompile_pc, cache_list[index].c8_end_recompile_pc,
		(uint32_t)cache_list[index].x86_mem_address, cache_list[index].x86_pc);
	printf("                         invalid_flag = %d, stop_write_flag = %d\n",
		cache_list[index].invalid_flag, cache_list[index].stop_write_flag);
}

void Chip8Engine_CacheHandler::DEBUG_printCacheList()
{
	for (uint32_t i = 0; i < cache_list.size(); i++) {
		printf("CacheHandler: Cache[%d]: C8_start_pc = 0x%.4X, C8_end_pc = 0x%.4X, X86_mem_address = 0x%.8X, X86_pc = 0x%.8X", 
			i, cache_list[i].c8_start_recompile_pc, cache_list[i].c8_end_recompile_pc, 
			(uint32_t)cache_list[i].x86_mem_address, cache_list[i].x86_pc);
		printf(", invalid_flag = %d, stop_write_flag = %d\n",
			cache_list[i].invalid_flag, cache_list[i].stop_write_flag);	
	}
}

void Chip8Engine_CacheHandler::incrementCacheX86PC(uint8_t count)
{
	cache_list[selected_cache_index].x86_pc += count;
}

void Chip8Engine_CacheHandler::write8(uint8_t byte_)
{
	*(cache_list[selected_cache_index].x86_mem_address + cache_list[selected_cache_index].x86_pc) = byte_;
	// DEBUG
	//printf("CacheHandler: Byte written:\t cache[%d] @ %.8X and value: 0x%.2X\n", selected_cache_index, memory_maps[selected_cache_index].x86_pc, byte_);
	incrementCacheX86PC(1);
}

void Chip8Engine_CacheHandler::write16(uint16_t word_)
{
	uint8_t* cache_mem_current = cache_list[selected_cache_index].x86_mem_address + cache_list[selected_cache_index].x86_pc;
	*((uint16_t*)cache_mem_current) = word_;
	// DEBUG
	//printf("CacheHandler: Word written:\t cache[%d] @ %.8X and value: 0x%.4X\n", selected_cache_index, memory_maps[selected_cache_index].x86_pc, word_);
	incrementCacheX86PC(2);
}

void Chip8Engine_CacheHandler::write32(uint32_t dword_)
{
	uint8_t* cache_mem_current = cache_list[selected_cache_index].x86_mem_address + cache_list[selected_cache_index].x86_pc;
	*((uint32_t*)cache_mem_current) = dword_;
	// DEBUG
	//printf("CacheHandler: Dword written:\t cache[%d] @ %.8X and value: 0x%.8X\n", selected_cache_index, memory_maps[selected_cache_index].x86_pc, dword_);
	incrementCacheX86PC(4);
}
