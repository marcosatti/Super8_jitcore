#include "stdafx.h"

#include <cstdint>

#include "Headers\Globals.h"

#include "Headers\Chip8Globals\MainEngineGlobals.h"
#include "Headers\Chip8Engine\CodeEmitter_x86.h"
#include "Headers\Chip8Engine\CacheHandler.h"

using namespace Chip8Globals::MainEngineGlobals;
using namespace Chip8Engine;

void CodeEmitter_x86::SUB_MfromR_8(X86Register dest, uint8_t* source)
{
	cachehandler->write8(0x2A);
	cachehandler->write8(ModRegRM(0, dest, (X86Register)MODREGRM_RM_DISP32));
	cachehandler->write32((uint32_t)source);
}

void CodeEmitter_x86::SUB_ImmfromR_8(X86Register dest, uint8_t immediate)
{
	cachehandler->write8(0x80);
	cachehandler->write8(ModRegRM(3, (X86Register)5, dest));
	cachehandler->write8(immediate);
}
