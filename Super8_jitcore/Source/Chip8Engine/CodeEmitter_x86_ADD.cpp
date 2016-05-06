#include "stdafx.h"

#include <cstdint>

#include "Headers\Globals.h"

#include "Headers\Chip8Globals\MainEngineGlobals.h"
#include "Headers\Chip8Engine\CodeEmitter_x86.h"
#include "Headers\Chip8Engine\CacheHandler.h"

using namespace Chip8Globals::MainEngineGlobals;
using namespace Chip8Engine;

void CodeEmitter_x86::ADD_MtoR_8(X86Register dest, uint8_t* source)
{
	cachehandler->write8(0x02);
	cachehandler->write8(ModRegRM(0, dest, (X86Register)MODREGRM_RM_DISP32));
	cachehandler->write32((uint32_t)source);
}

void CodeEmitter_x86::ADD_RtoR_8(X86Register dest, X86Register source)
{
	cachehandler->write8(0x00);
	cachehandler->write8(ModRegRM(3, source, dest));
}

void CodeEmitter_x86::ADD_ImmtoR_8(X86Register dest, uint8_t immediate)
{
	cachehandler->write8(0x80);
	cachehandler->write8(ModRegRM(3, (X86Register)0, dest));
	cachehandler->write8(immediate);
}

void CodeEmitter_x86::ADD_ImmtoM_8(uint8_t* dest, uint8_t immediate)
{
	cachehandler->write8(0x80);
	cachehandler->write8(ModRegRM(0, (X86Register)0, (X86Register)MODREGRM_RM_DISP32));
	cachehandler->write32((uint32_t)dest);
	cachehandler->write8(immediate);
}

void CodeEmitter_x86::ADD_RtoM_16(uint16_t * dest, X86Register source)
{
	cachehandler->write8(0x66);
	cachehandler->write8(0x01);
	cachehandler->write8(ModRegRM(0, source, (X86Register)MODREGRM_RM_DISP32));
	cachehandler->write32((uint32_t)dest);
}

void CodeEmitter_x86::ADD_MtoR_16(X86Register dest, uint16_t * source)
{
	cachehandler->write8(0x66);
	cachehandler->write8(0x03);
	cachehandler->write8(ModRegRM(0, dest, (X86Register)MODREGRM_RM_DISP32));
	cachehandler->write32((uint32_t)source);
}

void CodeEmitter_x86::ADD_ImmtoR_32(X86Register dest, uint32_t immediate)
{
	cachehandler->write8(0x81);
	cachehandler->write8(ModRegRM(3, (X86Register)0, dest));
	cachehandler->write32(immediate);
}

void CodeEmitter_x86::ADD_RtoR_16(X86Register dest, X86Register source)
{
	cachehandler->write8(0x66);
	cachehandler->write8(0x01);
	cachehandler->write8(ModRegRM(3, source, dest));
}

void CodeEmitter_x86::ADD_ImmtoM_16(uint16_t * dest, uint16_t immediate)
{
	cachehandler->write8(0x66);
	cachehandler->write8(0x81);
	cachehandler->write8(ModRegRM(0, (X86Register)0, (X86Register)MODREGRM_RM_DISP32));
	cachehandler->write32((uint32_t)dest);
	cachehandler->write32(immediate);
}