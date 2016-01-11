#include "stdafx.h"

#include <cstdint>

#include "Headers\Globals.h"

#include "Headers\Chip8Globals\Chip8Globals.h"
#include "Headers\Chip8Engine\Chip8Engine_CodeEmitter_x86.h"
#include "Headers\Chip8Engine\Chip8Engine_CacheHandler.h"

using namespace Chip8Globals;

void Chip8Engine_CodeEmitter_x86::MOV_RtoM_8(uint8_t* dest, X86Register source)
{
	cache->write8(0x88);
	cache->write8(ModRegRM(0, source, (X86Register)MODREGRM_RM_DISP32));
	cache->write32((uint32_t)dest);
}

void Chip8Engine_CodeEmitter_x86::MOV_RtoM_32(uint32_t* dest, X86Register source)
{
	cache->write8(0x89);
	cache->write8(ModRegRM(0, source, (X86Register)MODREGRM_RM_DISP32));
	cache->write32((uint32_t)dest);
}

void Chip8Engine_CodeEmitter_x86::MOV_MtoR_8(X86Register dest, uint8_t* source)
{
	cache->write8(0x8A);
	cache->write8(ModRegRM(0, dest, (X86Register)MODREGRM_RM_DISP32));
	cache->write32((uint32_t)source);
}

void Chip8Engine_CodeEmitter_x86::MOV_MtoR_32(X86Register dest, uint32_t* source)
{
	cache->write8(0x8B);
	cache->write8(ModRegRM(0, dest, (X86Register)MODREGRM_RM_DISP32));
	cache->write32((uint32_t)source);
}

void Chip8Engine_CodeEmitter_x86::MOV_PTRtoR_8(X86Register dest, X86Register PTR_source)
{
	cache->write8(0x8A);
	cache->write8(ModRegRM(0, dest, PTR_source));
}

void Chip8Engine_CodeEmitter_x86::MOV_RtoPTR_8(X86Register PTR_dest, X86Register source)
{
	cache->write8(0x88);
	cache->write8(ModRegRM(0, source, PTR_dest));
}

void Chip8Engine_CodeEmitter_x86::MOV_RtoR_8(X86Register dest, X86Register source)
{
	cache->write8(0x88);
	cache->write8(ModRegRM(3, source, dest));
}

void Chip8Engine_CodeEmitter_x86::MOV_RtoR_32(X86Register dest, X86Register source)
{
	cache->write8(0x89);
	cache->write8(ModRegRM(3, source, dest));
}

void Chip8Engine_CodeEmitter_x86::MOV_ImmtoR_8(X86Register dest, uint8_t immediate)
{
	cache->write8(0xB0 + (uint8_t)dest);
	cache->write8(immediate);
}

void Chip8Engine_CodeEmitter_x86::MOV_ImmtoR_32(X86Register dest, uint32_t immediate)
{
	cache->write8(0xB8 + (uint8_t)dest);
	cache->write32(immediate);
}

void Chip8Engine_CodeEmitter_x86::MOV_ImmtoM_8(uint8_t* dest, uint8_t immediate)
{
	cache->write8(0xC6);
	cache->write8(ModRegRM(0, (X86Register)0, (X86Register)5));
	cache->write32((uint32_t)dest);
	cache->write8(immediate);
}

void Chip8Engine_CodeEmitter_x86::MOV_ImmtoM_32(uint32_t * dest, uint32_t immediate)
{
	cache->write8(0xC7);
	cache->write8(ModRegRM(0, (X86Register)0, (X86Register)5));
	cache->write32((uint32_t)dest);
	cache->write32(immediate);
}

void Chip8Engine_CodeEmitter_x86::MOV_RtoM_16(uint16_t * dest, X86Register source)
{
	cache->write8(0x66);
	cache->write8(0x89);
	cache->write8(ModRegRM(0, source, (X86Register)MODREGRM_RM_DISP32));
	cache->write32((uint32_t)dest);
}

void Chip8Engine_CodeEmitter_x86::MOV_MtoR_16(X86Register dest, uint16_t * source)
{
	cache->write8(0x66);
	cache->write8(0x8B);
	cache->write8(ModRegRM(0, dest, (X86Register)MODREGRM_RM_DISP32));
	cache->write32((uint32_t)source);
}

void Chip8Engine_CodeEmitter_x86::MOV_RtoR_16(X86Register dest, X86Register source)
{
	cache->write8(0x66);
	cache->write8(0x89);
	cache->write8(ModRegRM(3, source, dest));
}

void Chip8Engine_CodeEmitter_x86::MOV_ImmtoR_16(X86Register dest, uint16_t immediate)
{
	cache->write8(0x66);
	cache->write8(0xB8 + (uint8_t)dest);
	cache->write16(immediate);
}

void Chip8Engine_CodeEmitter_x86::MOV_ImmtoM_16(uint16_t * dest, uint16_t immediate)
{
	cache->write8(0x66);
	cache->write8(0xC7);
	cache->write8(ModRegRM(0, (X86Register)0, (X86Register)5));
	cache->write32((uint32_t)dest);
	cache->write16(immediate);
}