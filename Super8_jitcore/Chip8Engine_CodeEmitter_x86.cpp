#include "stdafx.h"
#include "Chip8Engine_CodeEmitter_x86.h"

using namespace Chip8Globals;
using namespace Chip8Globals::X86_STATE;

Chip8Engine_CodeEmitter_x86::Chip8Engine_CodeEmitter_x86()
{
}

Chip8Engine_CodeEmitter_x86::~Chip8Engine_CodeEmitter_x86()
{
}

void Chip8Engine_CodeEmitter_x86::DYNAREC_EMIT_INTERRUPT(Chip8Globals::X86_STATE::X86_INT_STATUS_CODE code, uint16_t c8_opcode)
{
	MOV_ImmtoM_8((uint8_t *)(&x86_interrupt_status_code), code); // Store status code into global variable.
	MOV_ImmtoM_16(&x86_interrupt_c8_param1, c8_opcode); // Store optional parameter c8_opcode into global variable. USE 0xFFFF IF NOT NEEDED.
	DYNAREC_EMIT_MOV_EAX_EIP(); // move current eip into eax (uses a 32-bit hack method)
	ADD_ImmtoR_32(eax, 18); // 18 is length of next two x86 opcodes
	DYNAREC_EMIT_RETURN_CDECL_JUMP();
}

void Chip8Engine_CodeEmitter_x86::DYNAREC_EMIT_INTERRUPT(X86_INT_STATUS_CODE code, uint16_t c8_opcode, uint16_t c8_return_pc)
{
	MOV_ImmtoM_8((uint8_t *)(&x86_interrupt_status_code), code); // Store status code into global variable.
	MOV_ImmtoM_16(&x86_interrupt_c8_param1, c8_opcode); // Store optional parameter c8_opcode into global variable. USE 0xFFFF IF NOT NEEDED.
	MOV_ImmtoM_16(&x86_interrupt_c8_param2, c8_return_pc); // Used with stack interrupts, contains location of return c8 pc (in 0x2NNN calls)
	DYNAREC_EMIT_MOV_EAX_EIP(); // move current eip into eax (uses a 32-bit hack method)
	ADD_ImmtoR_32(eax, 18); // 18 is length of next two x86 opcodes
	DYNAREC_EMIT_RETURN_CDECL_JUMP();
}

void Chip8Engine_CodeEmitter_x86::DYNAREC_EMIT_MOV_EAX_EIP()
{
	// Stores EIP into eax using a special hack in 32 bit mode.
	emitter->CALL_M_PTR_32((uint32_t*)&cache->setup_cache_eip_hack);
}

void Chip8Engine_CodeEmitter_x86::DYNAREC_EMIT_RETURN_CDECL_JUMP()
{
	MOV_RtoM_32(((uint32_t*)&x86_resume_address), eax); // Store return address in global var
	JMP_M_PTR_32((uint32_t*)&cache->setup_cache_return_jmp_address); // jump to return address in cdecl setup cache (for pop/push cleanup)
}

void Chip8Engine_CodeEmitter_x86::MUL_RwithR_8(X86Register source)
{
	// AX = AL * source reg
	cache->write8(0xF6);
	cache->write8(ModRegRM(3, (X86Register)4, source));
}

void Chip8Engine_CodeEmitter_x86::DIV_RwithR_8(X86Register source)
{
	// Unsigned divide AX by r / m8, with result stored in AL = Quotient, AH = Remainder. Eg: 16/3 -> quotient = 5, remainder = 1
	cache->write8(0xF6);
	cache->write8(ModRegRM(3, (X86Register)6, source));
}

void Chip8Engine_CodeEmitter_x86::CALL_M_PTR_32(uint32_t * ptr_address)
{
	cache->write8(0xFF);
	cache->write8(ModRegRM(0, (X86Register)2, (X86Register)MODREGRM_RM_DISP32));
	cache->write32((uint32_t)ptr_address);
}

void Chip8Engine_CodeEmitter_x86::RET()
{
	cache->write8(0xC3);
}

void Chip8Engine_CodeEmitter_x86::POP(X86Register reg)
{
	uint8_t opcode = 0x58 + (uint8_t)reg;
	cache->write8(opcode);
}

void Chip8Engine_CodeEmitter_x86::PUSH(X86Register reg)
{
	uint8_t opcode = 0x50 + (uint8_t)reg;
	cache->write8(opcode);
}

void Chip8Engine_CodeEmitter_x86::RDTSC()
{
	cache->write8(0x0F);
	cache->write8(0x31);
}

uint8_t Chip8Engine_CodeEmitter_x86::ModRegRM(uint8_t mod, X86Register reg, X86Register rm)
{
	return((mod << 6) | ((uint8_t)reg << 3) | (uint8_t)rm);
}

uint8_t Chip8Engine_CodeEmitter_x86::SclIdxBase(uint8_t scale, X86Register index, X86Register base)
{
	return((scale << 6) | ((uint8_t)index << 3) | (uint8_t)base);
}