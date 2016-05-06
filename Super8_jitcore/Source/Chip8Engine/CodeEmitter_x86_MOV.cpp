#include "stdafx.h"

#include <cstdint>

#include "Headers\Globals.h"

#include "Headers\Chip8Globals\MainEngineGlobals.h"
#include "Headers\Chip8Engine\CodeEmitter_x86.h"
#include "Headers\Chip8Engine\CacheHandler.h"

using namespace Chip8Globals::MainEngineGlobals;
using namespace Chip8Engine;

void CodeEmitter_x86::MOV_RtoM_8(uint8_t* dest, X86Register source)
{
	cachehandler->write8(0x88);
	cachehandler->write8(ModRegRM(0, source, (X86Register)MODREGRM_RM_DISP32));
	cachehandler->write32((uint32_t)dest);
}

void CodeEmitter_x86::MOV_MtoR_8(X86Register dest, uint8_t* source)
{
	cachehandler->write8(0x8A);
	cachehandler->write8(ModRegRM(0, dest, (X86Register)MODREGRM_RM_DISP32));
	cachehandler->write32((uint32_t)source);
}

void CodeEmitter_x86::MOV_PTRtoR_8(X86Register dest, X86Register PTR_source)
{
	cachehandler->write8(0x8A);
	cachehandler->write8(ModRegRM(0, dest, PTR_source));
}

void CodeEmitter_x86::MOV_RtoPTR_8(X86Register PTR_dest, X86Register source)
{
	cachehandler->write8(0x88);
	cachehandler->write8(ModRegRM(0, source, PTR_dest));
}

void CodeEmitter_x86::MOV_RtoM_32(uint32_t * dest, X86Register source)
{
	cachehandler->write8(0x89);
	cachehandler->write8(ModRegRM(0, source, (X86Register)MODREGRM_RM_DISP32));
	cachehandler->write32((uint32_t)dest);
}

void CodeEmitter_x86::MOV_ImmtoR_8(X86Register dest, uint8_t immediate)
{
	cachehandler->write8(0xB0 + (uint8_t)dest);
	cachehandler->write8(immediate);
}

void CodeEmitter_x86::MOV_ImmtoR_32(X86Register dest, uint32_t immediate)
{
	cachehandler->write8(0xB8 + (uint8_t)dest);
	cachehandler->write32(immediate);
}

void CodeEmitter_x86::MOV_ImmtoM_8(uint8_t* dest, uint8_t immediate)
{
	cachehandler->write8(0xC6);
	cachehandler->write8(ModRegRM(0, (X86Register)0, (X86Register)5));
	cachehandler->write32((uint32_t)dest);
	cachehandler->write8(immediate);
}

void CodeEmitter_x86::MOV_RtoM_16(uint16_t * dest, X86Register source)
{
	cachehandler->write8(0x66);
	cachehandler->write8(0x89);
	cachehandler->write8(ModRegRM(0, source, (X86Register)MODREGRM_RM_DISP32));
	cachehandler->write32((uint32_t)dest);
}

void CodeEmitter_x86::MOV_MtoR_16(X86Register dest, uint16_t * source)
{
	cachehandler->write8(0x66);
	cachehandler->write8(0x8B);
	cachehandler->write8(ModRegRM(0, dest, (X86Register)MODREGRM_RM_DISP32));
	cachehandler->write32((uint32_t)source);
}

void CodeEmitter_x86::MOV_ImmtoM_16(uint16_t * dest, uint16_t immediate)
{
	cachehandler->write8(0x66);
	cachehandler->write8(0xC7);
	cachehandler->write8(ModRegRM(0, (X86Register)0, (X86Register)5));
	cachehandler->write32((uint32_t)dest);
	cachehandler->write16(immediate);
}