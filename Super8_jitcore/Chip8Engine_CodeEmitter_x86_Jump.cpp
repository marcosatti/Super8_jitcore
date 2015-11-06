#include "stdafx.h"

#include "Chip8Engine_CodeEmitter_x86.h"

using namespace Chip8Globals;

void Chip8Engine_CodeEmitter_x86::JMP_REL_32(int32_t relative)
{
	cache->write8(0xE9);
	cache->write32(relative);
}

void Chip8Engine_CodeEmitter_x86::JMP_M_PTR_32(uint32_t * address)
{
	cache->write8(0xFF);
	cache->write8(ModRegRM(0, (X86Register)4, (X86Register)MODREGRM_RM_DISP32));
	cache->write32((uint32_t)address);
}

void Chip8Engine_CodeEmitter_x86::JC_8(int8_t relative)
{
	cache->write8(0x72);
	cache->write8(relative);
}

void Chip8Engine_CodeEmitter_x86::JC_32(int32_t relative)
{
	cache->write8(0x0F);
	cache->write8(0x82);
	cache->write32(relative);
}

void Chip8Engine_CodeEmitter_x86::JNC_8(int8_t relative)
{
	cache->write8(0x73);
	cache->write8(relative);
}

void Chip8Engine_CodeEmitter_x86::JNC_32(int32_t relative)
{
	cache->write8(0x0F);
	cache->write8(0x83);
	cache->write32(relative);
}

void Chip8Engine_CodeEmitter_x86::JE_8(int8_t relative)
{
	cache->write8(0x74);
	cache->write8(relative);
}

void Chip8Engine_CodeEmitter_x86::JE_32(int32_t relative)
{
	cache->write8(0x0F);
	cache->write8(0x84);
	cache->write32(relative);
}

void Chip8Engine_CodeEmitter_x86::JNE_8(int8_t relative)
{
	cache->write8(0x75);
	cache->write8(relative);
}

void Chip8Engine_CodeEmitter_x86::JNE_32(int32_t relative)
{
	cache->write8(0x0F);
	cache->write8(0x85);
	cache->write32(relative);
}

void Chip8Engine_CodeEmitter_x86::JNG_8(int8_t relative)
{
	// Same as JLE opcode
	cache->write8(0x7E);
	cache->write8(relative);
}

void Chip8Engine_CodeEmitter_x86::JNG_32(int32_t relative)
{
	cache->write8(0x0F);
	cache->write8(0x8E);
	cache->write32(relative);
}
