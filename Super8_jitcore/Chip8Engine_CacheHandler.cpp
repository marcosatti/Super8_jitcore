#include "stdafx.h"
#include "Chip8Engine_CacheHandler.h"

using namespace Chip8Globals;

Chip8Engine_CacheHandler::Chip8Engine_CacheHandler()
{
	setup_cache_cdecl = NULL;
	memory_maps.reserve(1024);
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
	printf("\nCache: Attempting to execute cdecl cache. x86_resume_address = 0x%.8X (cache[%d])\n", (uint32_t)X86_STATE::x86_resume_address, cache->getMemoryRegionIndexByX86Address(X86_STATE::x86_resume_address));
	void(*exec)() = (void(*)())setup_cache_cdecl;
	exec();
}

int32_t Chip8Engine_CacheHandler::allocNewMemoryRegion()
{
	// NOTE: usable space is MAX_CACHE_SZ (4095) - 11. See below:
	// WIN32 specific
	uint8_t * cache_mem = (uint8_t *)VirtualAlloc(0, MAX_CACHE_SZ, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	// set to NOP 0x90
	memset(cache_mem, 0x90, MAX_CACHE_SZ);

	// set last memory bytes to OUT_OF_CODE interrupt
	// Emits change x86_status_code to 2 (out of code) then jump back to cdecl return address
	uint8_t bytes[] = {
		0xC6, // (0) MOV m, Imm8
		0b00000101, // (1) MOV m, Imm8
		0x00, // (2) PTR 32 
		0x00, // (3) PTR 32
		0x00, // (4) PTR 32
		0x00, // (4) PTR 32
		0x02, // (5) X86_STATUS_CODE = 2 (OUT_OF_CODE)
		0xFF, // (6) JMP PTR 32
		0b00100101, // (7) JMP PTR 32
		0x00, // (8) PTR 32
		0x00, // (9) PTR 32
		0x00, // (10) PTR 32
		0x00 // (11) PTR 32
	};
	uint32_t x86_status_code_address = (uint32_t)&(X86_STATE::x86_status_code);
	uint32_t cdecl_return_address = (uint32_t)&cache->setup_cache_return_jmp_address;
	*((uint32_t*)(bytes + 2)) = x86_status_code_address;
	*((uint32_t*)(bytes + 8)) = cdecl_return_address;
	memcpy(cache_mem + MAX_CACHE_SZ - 11, bytes, sizeof(bytes) / sizeof(bytes[0])); // Write this to last 11 bytes of cache
	
	// Contruct memory region
	MEM_REGION memoryblock;
	memoryblock.c8_start_recompile_pc = 0xFFFF;
	memoryblock.c8_end_recompile_pc = 0xFFFF;
	memoryblock.x86_mem_address = cache_mem;
	memoryblock.x86_pc = 0;
	memoryblock.invalid_flag = 0;
	memoryblock.stop_write_flag = 0;
	memory_maps.push_back(memoryblock);

	// DEBUG
	printf("CacheHandler: Cache[%d] allocated. Location and size: %p, %d, C8 Start PC = 0x%.4X\n", memory_maps.size() - 1, cache_mem, MAX_CACHE_SZ, memoryblock.c8_start_recompile_pc);

	return (memory_maps.size() - 1);
}

int32_t Chip8Engine_CacheHandler::allocAndSwitchNewMemoryRegion()
{
	uint32_t index = allocNewMemoryRegion();
	switchMemoryRegionByIndex(index);
	return index;
}

int32_t Chip8Engine_CacheHandler::checkMemoryRegionAllocatedByC8PC(uint16_t c8_pc_)
{
	int32_t index = -1;
	for (uint32_t i = 0; i < memory_maps.size(); i++) {
		if (c8_pc_ >= memory_maps[i].c8_start_recompile_pc && c8_pc_ <= memory_maps[i].c8_end_recompile_pc && memory_maps[i].invalid_flag == 0) {
			index = i;
			break;
		}
	}
	return index;
}

int32_t Chip8Engine_CacheHandler::checkAndSwitchMemoryRegionAllocatedByC8PC(uint16_t c8_pc_)
{
	for (uint32_t i = 0; i < memory_maps.size(); i++) {
		if (c8_pc_ >= memory_maps[i].c8_start_recompile_pc && c8_pc_ <= memory_maps[i].c8_end_recompile_pc) {
			selected_cache_index = i;
			C8_STATE::cpu.pc = memory_maps[selected_cache_index].c8_end_recompile_pc;
			break;
		}
	}
	return selected_cache_index;
}

int32_t Chip8Engine_CacheHandler::allocNewMemoryRegionByC8PC(uint16_t c8_start_pc_)
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

	MEM_REGION memoryblock;
	memoryblock.c8_start_recompile_pc = c8_start_pc_;
	memoryblock.c8_end_recompile_pc = c8_start_pc_; // most likely unknown at allocation, so set to parsed pc as well.
	memoryblock.x86_mem_address = cache_mem;
	memoryblock.x86_pc = 0;
	memoryblock.invalid_flag = 0;
	memoryblock.stop_write_flag = 0;
	memory_maps.push_back(memoryblock);

	// DEBUG
	printf("CacheHandler: Cache[%d] allocated. Location and size: %p, %d, C8 Start PC = 0x%.4X\n", memory_maps.size() - 1, cache_mem, MAX_CACHE_SZ, c8_start_pc_);

	return (memory_maps.size() - 1);
}

int32_t Chip8Engine_CacheHandler::allocAndSwitchNewMemoryRegionByC8PC(uint16_t c8_start_pc_)
{
	uint32_t index = allocNewMemoryRegionByC8PC(c8_start_pc_);
	switchMemoryRegionByIndex(index);
	return index;
}

void Chip8Engine_CacheHandler::invalidateMemoryRegion()
{
	printf("CacheHandler: Cache[%d] invalidated. C8 Start PC = 0x%.4X, C8 End PC = 0x%.4X\n", selected_cache_index, memory_maps[selected_cache_index].c8_start_recompile_pc, memory_maps[selected_cache_index].c8_end_recompile_pc);
	// Invalidates (deletes) the current memory region selected.
	VirtualFree(memory_maps[selected_cache_index].x86_mem_address, 0, MEM_RELEASE);
	memory_maps.erase(memory_maps.begin() + selected_cache_index);

	// Handle selected_cache_index changes (set to -1, will need to reselect later)
	selected_cache_index = -1;
	printf("               New selected_cache_index = %d\n", selected_cache_index);
}

void Chip8Engine_CacheHandler::invalidateAllMemoryRegionsByFlag()
{
	for (int32_t i = 0; i < (int32_t)memory_maps.size(); i++) {
		if (memory_maps[i].invalid_flag == 1) {
			printf("CacheHandler: Cache[%d] invalidated. C8 Start PC = 0x%.4X, C8 End PC = 0x%.4X\n", i, memory_maps[i].c8_start_recompile_pc, memory_maps[i].c8_end_recompile_pc);
			VirtualFree(memory_maps[i].x86_mem_address, 0, MEM_RELEASE);
			memory_maps.erase(memory_maps.begin() + i);

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
	}
}

void Chip8Engine_CacheHandler::invalidateMemoryRegionByC8PC(uint16_t c8_pc_)
{
	for (int32_t i = 0; i < (int32_t)memory_maps.size(); i++) {
		if (c8_pc_ >= memory_maps[i].c8_start_recompile_pc && c8_pc_ <= memory_maps[i].c8_end_recompile_pc) {
			printf("CacheHandler: Cache[%d] invalidated. C8 Start PC = 0x%.4X, C8 End PC = 0x%.4X\n", i, memory_maps[i].c8_start_recompile_pc, memory_maps[i].c8_end_recompile_pc);
			VirtualFree(memory_maps[i].x86_mem_address, 0, MEM_RELEASE);
			memory_maps.erase(memory_maps.begin() + i);

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

void Chip8Engine_CacheHandler::invalidateMemoryRegionByIndex(int32_t index)
{
	printf("CacheHandler: Cache[%d] invalidated. C8 Start PC = 0x%.4X, C8 End PC = 0x%.4X\n", index, memory_maps[index].c8_start_recompile_pc, memory_maps[index].c8_end_recompile_pc);
	VirtualFree(memory_maps[index].x86_mem_address, 0, MEM_RELEASE);
	memory_maps.erase(memory_maps.begin() + index);

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

void Chip8Engine_CacheHandler::setInvalidFlag(uint8_t value)
{
	memory_maps[selected_cache_index].invalid_flag = value;
}

void Chip8Engine_CacheHandler::setInvalidFlagByIndex(int32_t index, uint8_t value)
{
	memory_maps[index].invalid_flag = value;
}

uint8_t Chip8Engine_CacheHandler::getInvalidFlagByIndex(int32_t index)
{
	return memory_maps[index].invalid_flag;
}

void Chip8Engine_CacheHandler::stopWriteMemoryRegion()
{
	memory_maps[selected_cache_index].stop_write_flag = 1;
}

uint8_t Chip8Engine_CacheHandler::getStopWriteMemoryRegionByIndex(int32_t index)
{
	return memory_maps[index].stop_write_flag;
}

void Chip8Engine_CacheHandler::switchMemoryRegionByC8PC(uint16_t c8_pc_)
{
	for (uint32_t i = 0; i < memory_maps.size(); i++) {
		if (c8_pc_ >= memory_maps[i].c8_start_recompile_pc && c8_pc_ <= memory_maps[i].c8_end_recompile_pc && memory_maps[i].invalid_flag == 0) {
			selected_cache_index = i;
			// set C8 pc to end of memory region
			C8_STATE::cpu.pc = memory_maps[selected_cache_index].c8_end_recompile_pc;
			break;
		}
	}
}

int32_t Chip8Engine_CacheHandler::getMemoryRegionIndex()
{
	return selected_cache_index;
}

void Chip8Engine_CacheHandler::incrementCacheC8Region(uint16_t c8_pc_offset_)
{
	memory_maps[selected_cache_index].c8_end_recompile_pc += c8_pc_offset_;
}

void Chip8Engine_CacheHandler::switchMemoryRegionByIndex(uint32_t index)
{
	selected_cache_index = index;
}

void Chip8Engine_CacheHandler::setMemoryRegionC8EndPC(uint16_t c8_end_pc_)
{
	if (memory_maps[selected_cache_index].c8_start_recompile_pc == 0xFFFF) memory_maps[selected_cache_index].c8_start_recompile_pc = c8_end_pc_;
	memory_maps[selected_cache_index].c8_end_recompile_pc = c8_end_pc_;
}

void Chip8Engine_CacheHandler::setMemoryRegionC8EndPCByIndex(uint32_t index, uint16_t c8_end_pc_)
{
	if (memory_maps[index].c8_start_recompile_pc == 0xFFFF) memory_maps[index].c8_start_recompile_pc = c8_end_pc_;
	memory_maps[index].c8_end_recompile_pc = c8_end_pc_;
}

uint16_t Chip8Engine_CacheHandler::getMemoryRegionC8EndPC()
{
	return memory_maps[selected_cache_index].c8_end_recompile_pc;
}

uint8_t * Chip8Engine_CacheHandler::getMemoryRegionCurrentx86Address()
{
	uint8_t* cache_mem_current = memory_maps[selected_cache_index].x86_mem_address + memory_maps[selected_cache_index].x86_pc;
	return cache_mem_current;
}

int32_t Chip8Engine_CacheHandler::getMemoryRegionIndexByX86Address(uint8_t * x86_address)
{
	int32_t index = -1;
	for (uint32_t i = 0; i < memory_maps.size(); i++) {
		if (x86_address >= memory_maps[i].x86_mem_address && x86_address <= (memory_maps[i].x86_mem_address + memory_maps[i].x86_pc)) {
			index = i;
			break;
		}
	}
	return index;
}

MEM_REGION * Chip8Engine_CacheHandler::getMemoryRegionInfo()
{
	return &(memory_maps[selected_cache_index]);
}

MEM_REGION * Chip8Engine_CacheHandler::getMemoryRegionInfoByIndex(uint32_t index)
{
	return &(memory_maps[index]);
}

MEM_REGION * Chip8Engine_CacheHandler::getMemoryRegionInfoByC8PC(uint16_t c8_pc_)
{
	uint32_t idx = -1;
	for (uint32_t i = 0; i < memory_maps.size(); i++) {
		if (c8_pc_ >= memory_maps[i].c8_start_recompile_pc && c8_pc_ <= memory_maps[i].c8_end_recompile_pc) {
			idx = i;
			break;
		}
	}
	return &(memory_maps[idx]);
}

void Chip8Engine_CacheHandler::clearCache()
{
	memset(memory_maps[selected_cache_index].x86_mem_address, 0x90, MAX_CACHE_SZ);
	memory_maps[selected_cache_index].x86_pc = 0;

	// DEBUG
	printf("Cache: Cache[%d] cleared and cache_pc reset to 0.\n", selected_cache_index);
}

void Chip8Engine_CacheHandler::incrementCacheX86PC(uint8_t count)
{
	memory_maps[selected_cache_index].x86_pc += count;
}

void Chip8Engine_CacheHandler::write8(uint8_t byte_)
{
	*(memory_maps[selected_cache_index].x86_mem_address + memory_maps[selected_cache_index].x86_pc) = byte_;
	// DEBUG
	//printf("CacheHandler: Byte written:\t cache[%d] @ %.8X and value: 0x%.2X\n", selected_cache_index, memory_maps[selected_cache_index].x86_pc, byte_);
	incrementCacheX86PC(1);
}

void Chip8Engine_CacheHandler::write16(uint16_t word_)
{
	uint8_t* cache_mem_current = memory_maps[selected_cache_index].x86_mem_address + memory_maps[selected_cache_index].x86_pc;
	*((uint16_t*)cache_mem_current) = word_;
	// DEBUG
	//printf("CacheHandler: Word written:\t cache[%d] @ %.8X and value: 0x%.4X\n", selected_cache_index, memory_maps[selected_cache_index].x86_pc, word_);
	incrementCacheX86PC(2);
}

void Chip8Engine_CacheHandler::write32(uint32_t dword_)
{
	uint8_t* cache_mem_current = memory_maps[selected_cache_index].x86_mem_address + memory_maps[selected_cache_index].x86_pc;
	*((uint32_t*)cache_mem_current) = dword_;
	// DEBUG
	//printf("CacheHandler: Dword written:\t cache[%d] @ %.8X and value: 0x%.8X\n", selected_cache_index, memory_maps[selected_cache_index].x86_pc, dword_);
	incrementCacheX86PC(4);
}
