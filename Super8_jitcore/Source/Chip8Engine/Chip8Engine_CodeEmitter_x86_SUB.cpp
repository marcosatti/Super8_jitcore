#include "stdafx.h"

#include <cstdint>

#include "Headers\Globals.h"

#include "Headers\Chip8Globals\Chip8Globals.h"
#include "Headers\Chip8Engine\Chip8Engine_CodeEmitter_x86.h"
#include "Headers\Chip8Engine\Chip8Engine_CacheHandler.h"

using namespace Chip8Globals;

void Chip8Engine_CodeEmitter_x86::SUB_MfromR_8(X86Register dest, uint8_t* source)
{
	cache->write8(0x2A);
	cache->write8(ModRegRM(0, dest, (X86Register)MODREGRM_RM_DISP32));
	cache->write32((uint32_t)source);
}

void Chip8Engine_CodeEmitter_x86::SUB_ImmfromR_8(X86Register dest, uint8_t immediate)
{
	cache->write8(0x80);
	cache->write8(ModRegRM(3, (X86Register)5, dest));
	cache->write8(immediate);
}
