#include "stdafx.h"

#include <cstdint>

#include "Headers\Globals.h"

#include "Headers\Chip8Globals\MainEngineGlobals.h"
#include "Headers\Chip8Globals\X86_STATE.h"
#include "Headers\Chip8Engine\CodeEmitter_x86.h"
#include "Headers\Chip8Engine\CacheHandler.h"

using namespace Chip8Globals::X86_STATE;
using namespace Chip8Globals::MainEngineGlobals;
using namespace Chip8Engine;

CodeEmitter_x86::CodeEmitter_x86()
{
	// Register this component in logger
	logger->registerComponent(this);
}

CodeEmitter_x86::~CodeEmitter_x86()
{
	// Deregister this component in logger
	logger->deregisterComponent(this);
}

std::string CodeEmitter_x86::getComponentName()
{
	return std::string("CodeEmitter");
}

void CodeEmitter_x86::DYNAREC_EMIT_INTERRUPT(X86_INT_STATUS_CODE code, uint16_t c8_opcode)
{
	MOV_ImmtoM_8((uint8_t *)(&x86_interrupt_status_code), code); // Store status code into global variable (x86_resume_address).
	MOV_ImmtoM_16(&x86_interrupt_c8_param1, c8_opcode); // Store optional parameter c8_opcode into global variable. USE 0xFFFF IF NOT NEEDED.
	DYNAREC_EMIT_MOV_EAX_EIP(); // move current eip into eax (uses a 32-bit hack method)
	ADD_ImmtoR_32(eax, 18); // 18 is length of next two x86 opcodes
	DYNAREC_EMIT_RETURN_CDECL_JUMP();
}

void CodeEmitter_x86::DYNAREC_EMIT_INTERRUPT(X86_INT_STATUS_CODE code, uint16_t c8_opcode, uint16_t c8_return_pc)
{
	MOV_ImmtoM_8((uint8_t *)(&x86_interrupt_status_code), code); // Store status code into global variable (x86_resume_address).
	MOV_ImmtoM_16(&x86_interrupt_c8_param1, c8_opcode); // Store optional parameter c8_opcode into global variable. USE 0xFFFF IF NOT NEEDED.
	MOV_ImmtoM_16(&x86_interrupt_c8_param2, c8_return_pc); // Used with stack interrupts, contains location of return c8 pc (in 0x2NNN calls)
	DYNAREC_EMIT_MOV_EAX_EIP(); // move current eip into eax (uses a 32-bit hack method)
	ADD_ImmtoR_32(eax, 18); // 18 is length of next two x86 opcodes
	DYNAREC_EMIT_RETURN_CDECL_JUMP();
}

void CodeEmitter_x86::DYNAREC_EMIT_MOV_EAX_EIP()
{
	// Stores EIP into eax using a special hack in 32 bit mode.
	codeemitter_x86->CALL_M_PTR_32((uint32_t*)&cachehandler->setup_cache_cdecl_eip_hack);
}

void CodeEmitter_x86::DYNAREC_EMIT_RETURN_CDECL_JUMP()
{
	MOV_RtoM_32(((uint32_t*)&x86_resume_address), eax); // Store return address in global var
	JMP_M_PTR_32((uint32_t*)&cachehandler->setup_cache_cdecl_return_address); // jump to return address in cdecl setup cache (for pop/push cleanup)
}

void CodeEmitter_x86::MUL_RwithR_8(X86Register source)
{
	// AX = AL * source reg
	cachehandler->write8(0xF6);
	cachehandler->write8(ModRegRM(3, (X86Register)4, source));
}

void CodeEmitter_x86::DIV_RwithR_8(X86Register source)
{
	// Unsigned divide AX by r / m8, with result stored in AL = Quotient, AH = Remainder. Eg: 16/3 -> quotient = 5, remainder = 1
	cachehandler->write8(0xF6);
	cachehandler->write8(ModRegRM(3, (X86Register)6, source));
}

void CodeEmitter_x86::CALL_M_PTR_32(uint32_t * ptr_address)
{
	cachehandler->write8(0xFF);
	cachehandler->write8(ModRegRM(0, (X86Register)2, (X86Register)MODREGRM_RM_DISP32));
	cachehandler->write32((uint32_t)ptr_address);
}

void CodeEmitter_x86::RET()
{
	cachehandler->write8(0xC3);
}

void CodeEmitter_x86::POP(X86Register reg)
{
	uint8_t opcode = 0x58 + (uint8_t)reg;
	cachehandler->write8(opcode);
}

void CodeEmitter_x86::PUSH(X86Register reg)
{
	uint8_t opcode = 0x50 + (uint8_t)reg;
	cachehandler->write8(opcode);
}

void CodeEmitter_x86::RDTSC()
{
	cachehandler->write8(0x0F);
	cachehandler->write8(0x31);
}

uint8_t CodeEmitter_x86::ModRegRM(uint8_t mod, X86Register reg, X86Register rm)
{
	return((mod << 6) | ((uint8_t)reg << 3) | (uint8_t)rm);
}