#include "stdafx.h"
#include "Chip8Engine_CacheHandler.h"

using namespace Chip8Globals;

Chip8Engine_CacheHandler::Chip8Engine_CacheHandler()
{
	cache_list = new FastArrayList<CACHE_REGION>(1024);
	cache_invalidate_list = new FastArrayList<int32_t>(1024);
	setup_cache_cdecl = NULL;
}

Chip8Engine_CacheHandler::~Chip8Engine_CacheHandler()
{
	deallocAllCacheExit();
	delete cache_list;
}

void Chip8Engine_CacheHandler::setupCache_CDECL()
{
	// A small cache which is used to handle the CDECL call convention before passing off to the main cache execution point.
	// Also contains the x86 EIP hack used to get the current EIP address and store it in the eax register.
	if (setup_cache_cdecl == NULL) {
		// Alloc cdecl setup cache for first time. Will not change after this.
		uint8_t	bytes[] = {
			// Below code is used to 1. start CDECL calling convention, 2. goto emulation resume point, then 3. cleanup (return point).
			// 1.
			0x55,					//0x0 PUSH ebp
			0x89,					//0x1 (1) MOV ebp, esp
			0b11100101,				//0x2 (2, MODRM) MOV ebp, esp

			// 2.
			0xFF,					//0x3 (1) JMP r/m32
			0b00100101,				//0x4 (2, MODRM) JMP r/m32
			0x00,					//0x5 (3, DISP32)
			0x00,					//0x6 (4, DISP32)
			0x00,					//0x7 (5, DISP32)
			0x00,					//0x8 (6, DISP32)

			// 3.
			0x5D,					//0x9 POP ebp
			0xC3,					//0xA RET

			// HACK: ASM BELOW USED TO GET EIP ADDRESS AND RETURN IN EAX. SEE CodeEmitter_x86->DYNAREC_MOV_EAX_EIP.
			0x58,					//0xB POP eax
			0x50,					//0xC PUSH eax
			0xC3					//0xD RET
		};
		setup_cache_cdecl_sz = sizeof(bytes) / sizeof(bytes[0]);

		// WIN32 specific
		if ((setup_cache_cdecl = (uint8_t *)VirtualAlloc(0, setup_cache_cdecl_sz, MEM_COMMIT, PAGE_EXECUTE_READWRITE)) == NULL) exit(2);

		// Copy above raw x86 code into executable memory page.
		memcpy(setup_cache_cdecl, bytes, setup_cache_cdecl_sz);

		// Update variables needed throughout program.
		setup_cache_return_jmp_address = (setup_cache_cdecl + 0x9);
		setup_cache_eip_hack = (setup_cache_cdecl + 0xB);

		// Update cdecl cache with location of x86_resume_address variable (will jump to address contained in x86_resume_address).
		*(uint32_t *)(setup_cache_cdecl + 0x5) = (uint32_t)&X86_STATE::x86_resume_address;

		// DEBUG
#ifdef USE_VERBOSE
		printf("Cache: CDECL Cache allocated. Location and size: 0x%.8X, %d\n", (uint32_t)setup_cache_cdecl, setup_cache_cdecl_sz);
		printf("       setup_cache_return_jmp_address @ location 0x%.8X\n", (uint32_t)&setup_cache_return_jmp_address);
		printf("       setup_cache_eip_hack @ location 0x%.8X\n", (uint32_t)&setup_cache_eip_hack);
		printf("       x86_resume_address @ location 0x%.8X\n", (uint32_t)&X86_STATE::x86_resume_address);
#endif
	}
}

void Chip8Engine_CacheHandler::execCache_CDECL()
{
	// Old method, doesnt work with optimisations turned on (optimises to JMP instead of CALL).
	//void(__cdecl *exec)() = (void(__cdecl *)())setup_cache_cdecl;
	//exec();

	// New method, works with optimisations turned on. TODO: Look at why we cant direcly place value of setup_cdecl_cache into eax and call.. seems to put 14h instead of address.. probably something to do with stack.
	// CDECL calling convention, but there are no variables to push onto stack/remove from stack by changing esp.
	uint32_t call_address = (uint32_t)&setup_cache_cdecl;
	__asm {
		mov eax, call_address
		call [eax]
	};
}

void Chip8Engine_CacheHandler::initFirstCache()
{
	allocAndSwitchNewCacheByC8PC(C8_STATE::cpu.pc);
	X86_STATE::x86_resume_address = getCacheInfoCurrent()->x86_mem_address;
}

int32_t Chip8Engine_CacheHandler::findCacheIndexByC8PC(uint16_t c8_pc_)
{
	int32_t index = -1;
	for (int32_t i = 0; i < cache_list->size(); i++) {
		if (c8_pc_ >= cache_list->get_ptr(i)->c8_start_recompile_pc
			&& c8_pc_ <= cache_list->get_ptr(i)->c8_end_recompile_pc
			&& cache_invalidate_list->find(i) == -1
			&& cache_list->get_ptr(i)->c8_pc_alignement == C8_STATE::C8_getPCByteAlignmentOffset(c8_pc_)) { // Need to also check for pc alignment 5/11/15
			index = i;
			break;
		}
	}
	return index;
}

int32_t Chip8Engine_CacheHandler::findCacheIndexByStartC8PC(uint16_t c8_pc_)
{
	int32_t index = -1;
	for (int32_t i = 0; i < cache_list->size(); i++) {
		if (c8_pc_ == cache_list->get_ptr(i)->c8_start_recompile_pc 
			&& cache_invalidate_list->find(i) == -1) { // dont need to check for pc_alignement as its already defined to be aligned by checking against the starting pc
			index = i;
			break;
		}
	}
	return index;
}

int32_t Chip8Engine_CacheHandler::findCacheIndexByX86Address(uint8_t * x86_address)
{
	uint8_t * x86_end = NULL;
	for (int32_t i = 0; i < cache_list->size(); i++) {
		x86_end = cache_list->get_ptr(i)->x86_mem_address + cache_list->get_ptr(i)->x86_pc;
		if (x86_address >= cache_list->get_ptr(i)->x86_mem_address && x86_address <= x86_end) {
			return i;
		}
	}
	return -1;
}

int32_t Chip8Engine_CacheHandler::allocNewCacheByC8PC(uint16_t c8_start_pc_)
{
	// WIN32 specific
	uint8_t * cache_mem = NULL;
	if ((cache_mem = (uint8_t *)VirtualAlloc(0, MAX_CACHE_SZ, MEM_COMMIT, PAGE_EXECUTE_READWRITE)) == NULL) exit(2); // Do not continue if this does not work. Critical part of app.

	// set to NOP 0x90
	memset(cache_mem, 0x90, MAX_CACHE_SZ);
	
	// set last memory bytes to OUT_OF_CODE interrupt
	// Emits change x86_status_code to 2 (out of code) & x86_resume_c8_pc = c8_start_pc then jump back to cdecl return address
	uint8_t bytes[] = {
		0xC6,		// (0) MOV m, Imm8
		0b00000101, // (1) MOV m, Imm8
		0x00,		// (2) PTR 32
		0x00,		// (3) PTR 32
		0x00,		// (4) PTR 32
		0x00,		// (5) PTR 32
		0x02,		// (6) X86_STATUS_CODE = 2 (OUT_OF_CODE)
		//-----------------------------------------------------
		0xC7,		// (7) MOV m, Imm32
		0b00000101, // (8) MOV m, Imm32
		0x00,		// (9) PTR 32
		0x00,		// (10) PTR 32
		0x00,		// (11) PTR 32
		0x00,		// (12) PTR 32
		0x00,		// (13) X86_RESUME_START_ADDRESS
		0x00,		// (14) X86_RESUME_START_ADDRESS
		0x00,		// (15) X86_RESUME_START_ADDRESS
		0x00,		// (16) X86_RESUME_START_ADDRESS
		//-----------------------------------------------------
		0xFF,		// (17) JMP PTR 32
		0b00100101, // (18) JMP PTR 32
		0x00,		// (19) PTR 32
		0x00,		// (20) PTR 32
		0x00,		// (21) PTR 32
		0x00		// (22) PTR 32
	};
	uint32_t x86_status_code_address = (uint32_t)&(X86_STATE::x86_interrupt_status_code);
	uint32_t x86_resume_start_address_ = (uint32_t)&(X86_STATE::x86_interrupt_x86_param1);
	uint32_t cdecl_return_address = (uint32_t)&cache->setup_cache_return_jmp_address;
	*((uint32_t*)(bytes + 2)) = x86_status_code_address;
	*((uint32_t*)(bytes + 9)) = x86_resume_start_address_;
	*((uint32_t*)(bytes + 13)) = (uint32_t)cache_mem;
	*((uint32_t*)(bytes + 19)) = cdecl_return_address;
	uint8_t sz = sizeof(bytes) / sizeof(bytes[0]);
	memcpy(cache_mem + MAX_CACHE_SZ - sz, bytes, sz); // Write this to last bytes of cache

	// cache end pc is unknown at allocation, so set to start pc too (it is known if its a new cache by checking start==end pc)
	CACHE_REGION memoryblock = { c8_start_pc_, c8_start_pc_, C8_STATE::C8_getPCByteAlignmentOffset(c8_start_pc_), cache_mem, 0, 0 };
	cache_list->push_back(memoryblock);

	// DEBUG
#ifdef USE_VERBOSE
	printf("CacheHandler: Cache[%d] allocated. Location and size: %p, %d, C8 Start PC = 0x%.4X\n", cache_list->size() - 1, cache_mem, MAX_CACHE_SZ, c8_start_pc_);
#endif
	return (cache_list->size() - 1);
}

int32_t Chip8Engine_CacheHandler::getCacheWritableByStartC8PC(uint16_t c8_jump_pc)
{
	int32_t index = cache->findCacheIndexByStartC8PC(c8_jump_pc);
	if (index == -1) {
		// No cache was found, so check if theres a cache with the pc in the middle
		index = findCacheIndexByC8PC(c8_jump_pc);
		if (index == -1) {
			// No cache was found at all, so allocate a completely new cache
			index = allocNewCacheByC8PC(c8_jump_pc);
			//printf("CacheHandler: Jump Cache Path Result = NEW CACHE(%d)\n", index);
		}
		else {
			// A cache was found where the jump pc is located in the middle of the cache.
			// Need to mark invalid and alloc a new cache, as this code path for the cache wont be run again (next time the new cache will be found)
			setInvalidFlagByIndex(index);
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

int32_t Chip8Engine_CacheHandler::getCacheWritableByC8PC(uint16_t c8_pc)
{
	// First check if memory region is ready/allocated (check for both current pc (indicates already recompiled), and check for pc-2 (ready to write to))
	int32_t index = findCacheIndexByC8PC(c8_pc);
	if (index != -1) {
		// Cache exists for current PC
		// Invalid check already done in the above function.
		// TODO: remove below if statement. Invalid check already performed in the above function.
		if (getInvalidFlagByIndex(index)) {
			// Cache was invalid, so create a new cache at current PC
			index = allocNewCacheByC8PC(c8_pc);
		}
		else {
			// Cache was not marked as invalid, so next check for the do not write flag.
			// However, it does not matter as later the pc will be moved to the end of the region (code must already exist in this logic block), so just switch to this cache for now
			// cache_index = cache_index;
		}
	}
	else {
		// Cache does not exist for current PC, so it hasnt been compiled before. Need to get region which is available to hold this recompiled code
		// Check for cache with PC - 2 end PC.
		index = findCacheIndexByC8PC(c8_pc - 2);
		if (index != -1) {
			// Cache exists for current PC - 2
			// Check if its invalid
			if (getInvalidFlagByIndex(index)) {
				// Cache was invalid, so create a new cache at current PC
				index = allocNewCacheByC8PC(c8_pc);
			}
			else {
				// Cache was not marked as invalid, so next check for the do not write flag.
				if (getStopWriteFlagByIndex(index)) {
					// Cache has do not write flag set, so allocate a new region
					index = allocNewCacheByC8PC(c8_pc);
				}
				else {
					// Cache does not have do not write flag set, so its safe to write to this cache
					// cache_index = cache_index;
				}
			}
		}
		else {
			// No cache was found for PC - 2 as well, so allocate a new one for current pc
			index = allocNewCacheByC8PC(c8_pc);
		}
	}
	return index;
}

int32_t Chip8Engine_CacheHandler::allocAndSwitchNewCacheByC8PC(uint16_t c8_start_pc_)
{
	uint32_t index = allocNewCacheByC8PC(c8_start_pc_);
	switchCacheByIndex(index);
	return index;
}

void Chip8Engine_CacheHandler::deallocAllCacheExit()
{
	for (int32_t i = 0; i < cache_list->size(); i++) {
#ifdef USE_VERBOSE
		printf("CacheHandler: Cache[%d] invalidated. C8 Start PC = 0x%.4X, C8 End PC = 0x%.4X\n", i, cache_list->get_ptr(i)->c8_start_recompile_pc, cache_list->get_ptr(i)->c8_end_recompile_pc);
#endif
		VirtualFree(cache_list->get_ptr(i)->x86_mem_address, 0, MEM_RELEASE);
	}
}

void Chip8Engine_CacheHandler::invalidateCacheByFlag()
{
	// Function designed to be fast as it will be called many times.
	int32_t list_sz = cache_invalidate_list->size();
	if (list_sz > 0) {
		int32_t cache_index;
		for (int32_t i = 0; i < list_sz; i++) {
			cache_index = cache_invalidate_list->get(i);
			if (!(X86_STATE::x86_resume_address >= cache_list->get_ptr(cache_index)->x86_mem_address && X86_STATE::x86_resume_address <= (cache_list->get_ptr(cache_index)->x86_mem_address + cache_list->get_ptr(cache_index)->x86_pc))) { // check to make sure the resume address is not currently inside this cache
																																												   // First remove any jump references to this cache
				jumptbl->clearFilledFlagByC8PC(cache_list->get_ptr(cache_index)->c8_start_recompile_pc);

				// Delete cache here
#ifdef USE_VERBOSE
				printf("CacheHandler: Cache[%d] invalidated. C8 Start PC = 0x%.4X, C8 End PC = 0x%.4X\n", cache_index, cache_list->get_ptr(cache_index)->c8_start_recompile_pc, cache_list->get_ptr(cache_index)->c8_end_recompile_pc);
#endif
				VirtualFree(cache_list->get_ptr(cache_index)->x86_mem_address, 0, MEM_RELEASE);
				cache_list->remove(cache_index);

				// Handle selected_cache_index changes
				if (selected_cache_index > cache_index) {
					// Need to decrease selected_cache by 1
					selected_cache_index -= 1;
				}
				//else if (selected_cache_index < i) {
				// Do nothing, we are ok
				//}
				else if (selected_cache_index == cache_index) {
					// Set to -1 (need to reselect later)
					selected_cache_index = -1;
				}
#ifdef USE_VERBOSE
				printf("               New selected_cache_index = %d\n", selected_cache_index);
#endif

				// remove entry after its been filled
				cache_invalidate_list->remove(i);
				list_sz = cache_invalidate_list->size(); // update list size again
				i -= 1; // decrease i by 1 so it rechecks the current i'th value in the list (which would have been i+1 if there was no remove).
			}
		}
	}
}

void Chip8Engine_CacheHandler::setInvalidFlagByIndex(int32_t index)
{
	cache_invalidate_list->push_back(index);
}

void Chip8Engine_CacheHandler::setInvalidFlagByC8PC(uint16_t c8_pc_)
{
	// Function designed to be fast, as it will be called many times
	for (int32_t i = 0; i < cache_list->size(); i++) {
		CACHE_REGION * cache = cache_list->get_ptr(i);
		if (c8_pc_ >= cache->c8_start_recompile_pc 
			&& c8_pc_ <= cache->c8_end_recompile_pc
			&& cache_invalidate_list->find(i) == -1
			&& cache->c8_pc_alignement == C8_STATE::C8_getPCByteAlignmentOffset(c8_pc_)) {
			setInvalidFlagByIndex(i);
			break;
		}
	}
}

uint8_t Chip8Engine_CacheHandler::getInvalidFlagByIndex(int32_t index)
{
	int32_t list_index;
	if ((list_index = cache_invalidate_list->find(index)) != -1) {
		return 1;
	}
	else {
		return 0;
	}
}

void Chip8Engine_CacheHandler::setStopWriteFlagCurrent()
{
	cache_list->get_ptr(selected_cache_index)->stop_write_flag = 1;
}

void Chip8Engine_CacheHandler::setStopWriteFlagByIndex(int32_t index)
{
	cache_list->get_ptr(index)->stop_write_flag = 1;
}

void Chip8Engine_CacheHandler::clearStopWriteFlagByIndex(int32_t index)
{
	cache_list->get_ptr(index)->stop_write_flag = 0;
}

uint8_t Chip8Engine_CacheHandler::getStopWriteFlagByIndex(int32_t index)
{
	return cache_list->get_ptr(index)->stop_write_flag;
}

void Chip8Engine_CacheHandler::switchCacheByC8PC(uint16_t c8_pc_)
{
	for (int32_t i = 0; i < cache_list->size(); i++) {
		if (c8_pc_ >= cache_list->get_ptr(i)->c8_start_recompile_pc 
			&& c8_pc_ <= cache_list->get_ptr(i)->c8_end_recompile_pc 
			&& cache_invalidate_list->find(i) == -1
			&& cache_list->get_ptr(i)->c8_pc_alignement == C8_STATE::C8_getPCByteAlignmentOffset(c8_pc_)) {
			selected_cache_index = i;
			// set C8 pc to end of memory region
			C8_STATE::cpu.pc = cache_list->get_ptr(selected_cache_index)->c8_end_recompile_pc;
			break;
		}
	}
}

int32_t Chip8Engine_CacheHandler::findCacheIndexCurrent()
{
	return selected_cache_index;
}

void Chip8Engine_CacheHandler::switchCacheByIndex(int32_t index)
{
	selected_cache_index = index;
}

void Chip8Engine_CacheHandler::setCacheEndC8PCCurrent(uint16_t c8_end_pc_)
{
	if (cache_list->get_ptr(selected_cache_index)->c8_start_recompile_pc == 0xFFFF) cache_list->get_ptr(selected_cache_index)->c8_start_recompile_pc = c8_end_pc_;
	cache_list->get_ptr(selected_cache_index)->c8_end_recompile_pc = c8_end_pc_;
}

void Chip8Engine_CacheHandler::setCacheEndC8PCByIndex(int32_t index, uint16_t c8_end_pc_)
{
	if (cache_list->get_ptr(index)->c8_start_recompile_pc == 0xFFFF) cache_list->get_ptr(index)->c8_start_recompile_pc = c8_end_pc_;
	cache_list->get_ptr(index)->c8_end_recompile_pc = c8_end_pc_;
}

uint16_t Chip8Engine_CacheHandler::getEndC8PCCurrent()
{
	return cache_list->get_ptr(selected_cache_index)->c8_end_recompile_pc;
}

uint8_t * Chip8Engine_CacheHandler::getEndX86AddressCurrent()
{
	uint8_t* cache_mem_current = cache_list->get_ptr(selected_cache_index)->x86_mem_address + cache_list->get_ptr(selected_cache_index)->x86_pc;
	return cache_mem_current;
}

CACHE_REGION * Chip8Engine_CacheHandler::getCacheInfoCurrent()
{
	return cache_list->get_ptr(selected_cache_index);
}

CACHE_REGION * Chip8Engine_CacheHandler::getCacheInfoByIndex(int32_t index)
{
	return cache_list->get_ptr(index);
}

CACHE_REGION * Chip8Engine_CacheHandler::getCacheInfoByC8PC(uint16_t c8_pc_)
{
	int32_t idx = -1;
	for (int32_t i = 0; i < cache_list->size(); i++) {
		if (c8_pc_ >= cache_list->get_ptr(i)->c8_start_recompile_pc 
			&& c8_pc_ <= cache_list->get_ptr(i)->c8_end_recompile_pc
			&& cache_list->get_ptr(i)->c8_pc_alignement == C8_STATE::C8_getPCByteAlignmentOffset(c8_pc_)) {
			idx = i;
			break;
		}
	}
	return cache_list->get_ptr(idx);
}

void Chip8Engine_CacheHandler::DEBUG_printCacheByIndex(int32_t index)
{
	printf("CacheHandler: Cache[%d]: C8_start_pc = 0x%.4X, C8_end_pc = 0x%.4X, X86_mem_address = 0x%.8X, X86_pc = 0x%.8X\n",
		index, cache_list->get_ptr(index)->c8_start_recompile_pc, cache_list->get_ptr(index)->c8_end_recompile_pc,
		(uint32_t)cache_list->get_ptr(index)->x86_mem_address, cache_list->get_ptr(index)->x86_pc);
	printf("                         invalid_flag = %d, stop_write_flag = %d\n",
		(cache_invalidate_list->find(index) != -1), cache_list->get_ptr(index)->stop_write_flag);
}

void Chip8Engine_CacheHandler::DEBUG_printCacheList()
{
	for (int32_t i = 0; i < cache_list->size(); i++) {
		printf("CacheHandler: Cache[%d]: C8_start_pc = 0x%.4X, C8_end_pc = 0x%.4X, X86_mem_address = 0x%.8X, X86_pc = 0x%.8X",
			i, cache_list->get_ptr(i)->c8_start_recompile_pc, cache_list->get_ptr(i)->c8_end_recompile_pc,
			(uint32_t)cache_list->get_ptr(i)->x86_mem_address, cache_list->get_ptr(i)->x86_pc);
		printf(", invalid_flag = %d, stop_write_flag = %d\n",
			(cache_invalidate_list->find(i) != -1), cache_list->get_ptr(i)->stop_write_flag);
	}
}

void Chip8Engine_CacheHandler::incrementCacheX86PC(uint8_t count)
{
	cache_list->get_ptr(selected_cache_index)->x86_pc += count;
}

void Chip8Engine_CacheHandler::write8(uint8_t byte_)
{
	*(cache_list->get_ptr(selected_cache_index)->x86_mem_address + cache_list->get_ptr(selected_cache_index)->x86_pc) = byte_;
	// DEBUG
	//printf("CacheHandler: Byte written:\t cache[%d] @ %.8X and value: 0x%.2X\n", selected_cache_index, memory_maps[selected_cache_index].x86_pc, byte_);
	incrementCacheX86PC(1);
}

void Chip8Engine_CacheHandler::write16(uint16_t word_)
{
	uint8_t* cache_mem_current = cache_list->get_ptr(selected_cache_index)->x86_mem_address + cache_list->get_ptr(selected_cache_index)->x86_pc;
	*((uint16_t*)cache_mem_current) = word_;
	// DEBUG
	//printf("CacheHandler: Word written:\t cache[%d] @ %.8X and value: 0x%.4X\n", selected_cache_index, memory_maps[selected_cache_index].x86_pc, word_);
	incrementCacheX86PC(2);
}

void Chip8Engine_CacheHandler::write32(uint32_t dword_)
{
	uint8_t* cache_mem_current = cache_list->get_ptr(selected_cache_index)->x86_mem_address + cache_list->get_ptr(selected_cache_index)->x86_pc;
	*((uint32_t*)cache_mem_current) = dword_;
	// DEBUG
	//printf("CacheHandler: Dword written:\t cache[%d] @ %.8X and value: 0x%.8X\n", selected_cache_index, memory_maps[selected_cache_index].x86_pc, dword_);
	incrementCacheX86PC(4);
}