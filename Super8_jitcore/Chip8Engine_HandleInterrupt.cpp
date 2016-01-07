#include "stdafx.h"
#include "Chip8Engine.h"

void Chip8Engine::handleInterrupt_PREPARE_FOR_JUMP()
{
	// ! ! ! X86_STATE::x86_interrupt_c8_param1 contains jump location ! ! !
	// Flush caches that are marked
	cache->invalidateCacheByFlag();

	// Need to update the jump table/cache before the jumps are made.
	//cache->DEBUG_printCacheList();
	jumptbl->checkAndFillJumpsByStartC8PC();
}

void Chip8Engine::handleInterrupt_USE_INTERPRETER()
{
	// ! ! ! X86_STATE::x86_interrupt_c8_param1 contains interpreter opcode ! ! !

	// Opcode hasnt been implemented in the dynarec yet, need to use interpreter
	interpreter->setOpcode(X86_STATE::x86_interrupt_c8_param1);
	interpreter->emulateCycle();
}

void Chip8Engine::handleInterrupt_OUT_OF_CODE()
{
	// ! ! ! X86_STATE::x86_interrupt_c8_param1 contains start pc of cache, X86_STATE::x86_resume_start_address contains starting x86 address of cache ! ! !
	// Flush caches that are marked
	cache->invalidateCacheByFlag();

	// End of cache was reached, because of 3 posibilities:
	// 1. The cache is genuinely out of code due to eg: translator loop exiting on X many cycles
	// 2. The cache contains code (ie: conditional jump) that goes over the end jump
	// 3. The cache has previously been split up into two caches due to a jump into the middle of the cache (and needs to be "joined up").

	// We can determine which case it is by looking at the do not write flag, where 1 and 3. will have false, and 2. will have true.
	// For determining between 1 and 3, we can check for an existing cache. If true, then it is case 3.
	// However, case 2 and 3 can be solved in the same way, by emitting a jump to (C8 end PC + 2).

	// In summary:
	// Case 1: stop_flag = 0, existing cache = 0
	// Case 2: stop_flag = 1, existing cache = 1 or 0
	// Case 3: stop_flag = 0, existing cache = 1
	// Together, Case 2 & 3 make up the complement possiblilies to Case 1 (ie: IF (Case 1) and ELSE (Case 2 & 3) statement will work).

	// PROBLEM:
	// There is sometimes a problem, where there has been a cache that has been split up where both of them are only 1 Chip8 opcode apart, eg: 0x206 and 0x208
	// In this case, the cache with start C8 pc = 0x206 will also have an end C8 pc = 0x206 but does not contain any translated code for the 0x206 PC.
	// This means that on the first OUT_OF_CODE interrupt from this cache, it will be detected as stop_flag = 0 and existing cache = 1 (Case 3), but actually it should be Case 1 as the code has not been translated yet.
	// After the code has been translated, thats when its safe to detect as Case 3.
	// The solution is to check within Case 1 if also no translated code exists by x86_pc == 0 (logical OR). Has not been thoroughly tested and may present problems in other games.
	
	// There is possibly a better way to do this...

#ifdef USE_DEBUG
	cache->DEBUG_printCacheList();
	printf("----\n\n");
#endif

	// Get cache details that caused interrupt.
	int32_t cache_index = cache->findCacheIndexByX86Address(X86_STATE::x86_interrupt_x86_param1); // param1 should be the base address of the cache, so we can find the cache that interrupted by searching for this value.
	CACHE_REGION * region = cache->getCacheInfoByIndex(cache_index);
	// Remember to reset the entry point to the current cache PC.
	X86_STATE::x86_resume_address = region->x86_mem_address + region->x86_pc;

	// Case logic
	// Case 1
	if (region->x86_pc == 0 
		|| (region->stop_write_flag == 0 
			&& cache->findCacheIndexByStartC8PC(region->c8_end_recompile_pc + 2) == -1)) {
		// Start recompiling code at end of cache c8pc
		C8_STATE::cpu.pc = region->c8_end_recompile_pc;
		translatorLoop();
	}
	// Case 2 & 3
	else {
		// First get jump table entry
		int32_t tblindex = jumptbl->getJumpIndexByC8PC(region->c8_end_recompile_pc + 2);

		// Emit the jump
		int32_t old_index = cache->findCacheIndexCurrent();
		cache->switchCacheByIndex(cache_index);
		emitter->DYNAREC_EMIT_INTERRUPT(X86_STATE::PREPARE_FOR_JUMP, region->c8_end_recompile_pc + 2);
		emitter->JMP_M_PTR_32((uint32_t*)&jumptbl->getJumpInfoByIndex(tblindex)->x86_address_to);
		cache->setStopWriteFlagCurrent();
		cache->switchCacheByIndex(old_index);
	}
}

void Chip8Engine::handleInterrupt_PREPARE_FOR_INDIRECT_JUMP()
{
	// ! ! ! X86_STATE::x86_interrupt_c8_param1 contains opcode ! ! !
	// Flush caches that are marked
	cache->invalidateCacheByFlag();

	// Need to update the jump table/cache before the jumps are made.
	//cache->DEBUG_printCacheList();
	switch (X86_STATE::x86_interrupt_c8_param1 & 0xF000) {
	case 0xB000: {
		uint16_t c8_address = X86_STATE::x86_interrupt_c8_param1 & 0x0FFF;
		c8_address += C8_STATE::cpu.V[0]; // get address to jump to
										  
		// Jump cache handling done by CacheHandler, so this function just updates the jump table locations
		int32_t cache_index = cache->getCacheWritableByStartC8PC(c8_address);
		CACHE_REGION * region = cache->getCacheInfoByIndex(cache_index);
		jumptbl->x86_indirect_jump_address = region->x86_mem_address;
	}
	}
	
}

void Chip8Engine::handleInterrupt_SELF_MODIFYING_CODE()
{
	// ! ! ! X86_STATE::x86_interrupt_c8_param1 contains opcode from translator ! ! !
	// Flush caches that are marked
	cache->invalidateCacheByFlag();

	// Only 2 opcodes in the C8 specs that do this. For SMC, need to invalidate cache that the memory writes to
	switch (X86_STATE::x86_interrupt_c8_param1 & 0xF0FF) {
	case 0xF033:
	{
		// 0xFX33: Splits the decimal representation of Vx into 3 locations: hundreds stored in address I, tens in address I+1, and ones in I+2.
		//cache->DEBUG_printCacheList();
		uint16_t I = C8_STATE::cpu.I;
		cache->setInvalidFlagByC8PC(C8_STATE::cpu.I);
		cache->setInvalidFlagByC8PC(C8_STATE::cpu.I + 1);
		cache->setInvalidFlagByC8PC(C8_STATE::cpu.I + 2);
		break;
	}
	case 0xF055:
	{
		// 0xFX55: Copies all current values in registers V0 -> Vx to memory starting at address I.
		uint8_t vx = (X86_STATE::x86_interrupt_c8_param1 & 0x0F00) >> 8; // Need to bit shift by 8 to get to a single base16 digit.
		for (uint8_t i = 0; i <= vx; i++) {
			cache->setInvalidFlagByC8PC(C8_STATE::cpu.I + i);
		}
		break;
	}
	}
}

#ifdef USE_DEBUG
void Chip8Engine::handleInterrupt_DEBUG()
{
	printf("\n!!! Debug Interrupt, Opcode = 0x%.4X, C8PC = 0x%.4X !!!\n", X86_STATE::x86_interrupt_c8_param1, X86_STATE::x86_interrupt_c8_param2);
	C8_STATE::DEBUG_printC8_STATE();
	//X86_STATE::DEBUG_printX86_STATE();
	//cache->DEBUG_printCacheList();
	//jumptbl->DEBUG_printJumpList();
	//jumptbl->DEBUG_printCondJumpList();
}
#endif

void Chip8Engine::handleInterrupt_WAIT_FOR_KEYPRESS()
{
	// ! ! ! X86_STATE::x86_interrupt_c8_param1 contains the C8 opcode ! ! !
	// Only one opcode: 0xFX0A: A key press is awaited, then stored in Vx.
	// For now this will do, however it should be handled by the parent object to the C8Engine
	// Check if there has been a key press, and if so, store it in key->x86_key_pressed
	uint8_t keystate = 0;
	for (int i = 0; i < NUM_KEYS; i++) {
		keystate = key->getKeyState(i); // Get the keystate from the key object.
		if (keystate == 1) {
			key->X86_KEY_PRESSED = i; // Set Vx to the key pressed (0x0 -> 0xF). See dynarec
			break;
		}
	}
}

void Chip8Engine::handleInterrupt_PREPARE_FOR_STACK_JUMP()
{
	// ! ! ! X86_STATE::x86_interrupt_c8_param1 contains either: 0x2NNN (call, address = NNN) or 0x00EE (ret) ! ! !
	// Flush caches that are marked
	cache->invalidateCacheByFlag();

	//cache->DEBUG_printCacheList();
	//stack->DEBUG_printStack();
	switch (X86_STATE::x86_interrupt_c8_param1 & 0xF000) {
	case 0x2000:
	{
		// get jump location & return location
		uint16_t jump_c8_pc = X86_STATE::x86_interrupt_c8_param1 & 0x0FFF;
		uint16_t return_c8_pc = X86_STATE::x86_interrupt_c8_param2;

		// Record stack entry for the return point - which will be the next opcode!
		STACK_ENTRY entry;
		entry.c8_address = return_c8_pc;
		stack->setTopStack(entry);

		// First get jump table entry
		int32_t tblindex = jumptbl->getJumpIndexByC8PC(jump_c8_pc);

		// Need to check/alloc jump location caches
		cache->getCacheWritableByStartC8PC(jump_c8_pc);
		jumptbl->checkAndFillJumpsByStartC8PC();

		// Set stack->x86_address_to equal to jumptable location
		stack->x86_address_to = jumptbl->getJumpInfoByIndex(tblindex)->x86_address_to;
		break;
	}
	case 0x0000:
	{
		// Get stack entry & set jump location
		STACK_ENTRY entry = stack->getTopStack();

		// First get jump table entry
		int32_t tblindex = jumptbl->getJumpIndexByC8PC(entry.c8_address);

		// Need to check/alloc jump location caches
		cache->getCacheWritableByStartC8PC(entry.c8_address);
		jumptbl->checkAndFillJumpsByStartC8PC();

		// Set stack->x86_address_to equal to jumptable location
		stack->x86_address_to = jumptbl->getJumpInfoByIndex(tblindex)->x86_address_to;
		break;
	}
	}
}