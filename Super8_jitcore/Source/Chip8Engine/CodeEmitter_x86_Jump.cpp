#include "stdafx.h"

#include <cstdint>

#include "Headers\Globals.h"

#include "Headers\Chip8Globals\MainEngineGlobals.h"
#include "Headers\Chip8Engine\CodeEmitter_x86.h"
#include "Headers\Chip8Engine\CacheHandler.h"

using namespace Chip8Globals::MainEngineGlobals;
using namespace Chip8Engine;

void CodeEmitter_x86::JMP_M_PTR_32(uint32_t * address)
{
	cachehandler->write8(0xFF);
	cachehandler->write8(ModRegRM(0, (X86Register)4, (X86Register)MODREGRM_RM_DISP32));
	cachehandler->write32((uint32_t)address);
}

void CodeEmitter_x86::JNC_8(int8_t relative)
{
	cachehandler->write8(0x73);
	cachehandler->write8(relative);
}

void CodeEmitter_x86::JE_32(int32_t relative)
{
	cachehandler->write8(0x0F);
	cachehandler->write8(0x84);
	cachehandler->write32(relative);
}

void CodeEmitter_x86::JNE_8(int8_t relative)
{
	cachehandler->write8(0x75);
	cachehandler->write8(relative);
}

void CodeEmitter_x86::JNE_32(int32_t relative)
{
	cachehandler->write8(0x0F);
	cachehandler->write8(0x85);
	cachehandler->write32(relative);
}

void CodeEmitter_x86::JNG_8(int8_t relative)
{
	// Same as JLE opcode
	cachehandler->write8(0x7E);
	cachehandler->write8(relative);
}