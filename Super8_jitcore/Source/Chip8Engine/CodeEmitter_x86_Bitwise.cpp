#include "stdafx.h"

#include <cstdint>

#include "Headers\Globals.h"

#include "Headers\Chip8Globals\MainEngineGlobals.h"
#include "Headers\Chip8Engine\CodeEmitter_x86.h"
#include "Headers\Chip8Engine\CacheHandler.h"

using namespace Chip8Globals::MainEngineGlobals;
using namespace Chip8Engine;

void CodeEmitter_x86::OR_RwithM_8(X86Register dest, uint8_t* source)
{
	cachehandler->write8(0x0A);
	cachehandler->write8(ModRegRM(0, dest, (X86Register)MODREGRM_RM_DISP32));
	cachehandler->write32((uint32_t)source);
}

void CodeEmitter_x86::AND_RwithM_8(X86Register dest, uint8_t * source)
{
	cachehandler->write8(0x22);
	cachehandler->write8(ModRegRM(0, dest, (X86Register)MODREGRM_RM_DISP32));
	cachehandler->write32((uint32_t)source);
}

void CodeEmitter_x86::XOR_RwithM_8(X86Register dest, uint8_t * source)
{
	cachehandler->write8(0x32);
	cachehandler->write8(ModRegRM(0, dest, (X86Register)MODREGRM_RM_DISP32));
	cachehandler->write32((uint32_t)source);
}

void CodeEmitter_x86::AND_RwithImm_8(X86Register dest, uint8_t immediate)
{
	cachehandler->write8(0x80);
	cachehandler->write8(ModRegRM(3, (X86Register)4, dest));
	cachehandler->write8(immediate);
}

void CodeEmitter_x86::XOR_RwithR_32(X86Register dest, X86Register source)
{
	cachehandler->write8(0x31);
	cachehandler->write8(ModRegRM(3, source, dest));
}

void CodeEmitter_x86::SHL_R_8(X86Register reg, uint8_t count)
{
	cachehandler->write8(0xC0);
	cachehandler->write8(ModRegRM(3, (X86Register)4, reg));
	cachehandler->write8(count);
}

void CodeEmitter_x86::SHR_R_8(X86Register reg, uint8_t count)
{
	cachehandler->write8(0xC0);
	cachehandler->write8(ModRegRM(3, (X86Register)5, reg));
	cachehandler->write8(count);
}

void CodeEmitter_x86::SHR_R_32(X86Register reg, uint8_t count)
{
	cachehandler->write8(0xC1);
	cachehandler->write8(ModRegRM(3, (X86Register)5, reg));
	cachehandler->write8(count);
}

void CodeEmitter_x86::CMP_RwithR_8(X86Register dest, X86Register source)
{
	cachehandler->write8(0x38);
	cachehandler->write8(ModRegRM(3, source, dest));
}

void CodeEmitter_x86::CMP_RwithImm_8(X86Register dest, uint8_t immediate)
{
	cachehandler->write8(0x80);
	cachehandler->write8(ModRegRM(3, (X86Register)7, dest));
	cachehandler->write8(immediate);
}

void CodeEmitter_x86::CMP_RwithImm_32(X86Register dest, uint32_t immediate)
{
	cachehandler->write8(0x81);
	cachehandler->write8(ModRegRM(3, (X86Register)7, dest));
	cachehandler->write32(immediate);
}