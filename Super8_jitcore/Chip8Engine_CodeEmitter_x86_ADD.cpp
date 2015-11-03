#include "stdafx.h"
#include "Chip8Engine_CodeEmitter_x86.h"

using namespace Chip8Globals;

void Chip8Engine_CodeEmitter_x86::ADD_RtoM_8(uint8_t* dest, X86Register source)
{
	cache->write8(0x00);
	cache->write8(ModRegRM(0, source, (X86Register)MODREGRM_RM_DISP32));
	cache->write32((uint32_t)dest);
}

void Chip8Engine_CodeEmitter_x86::ADD_RtoM_32(uint32_t* dest, X86Register source)
{
	cache->write8(0x01);
	cache->write8(ModRegRM(0, source, (X86Register)MODREGRM_RM_DISP32));
	cache->write32((uint32_t)dest);
}

void Chip8Engine_CodeEmitter_x86::ADD_MtoR_8(X86Register dest, uint8_t* source)
{
	cache->write8(0x02);
	cache->write8(ModRegRM(0, dest, (X86Register)MODREGRM_RM_DISP32));
	cache->write32((uint32_t)source);
}

void Chip8Engine_CodeEmitter_x86::ADD_MtoR_32(X86Register dest, uint32_t* source)
{
	cache->write8(0x03);
	cache->write8(ModRegRM(0, dest, (X86Register)MODREGRM_RM_DISP32));
	cache->write32((uint32_t)source);
}

void Chip8Engine_CodeEmitter_x86::ADD_RtoR_8(X86Register dest, X86Register source)
{
	cache->write8(0x00);
	cache->write8(ModRegRM(3, source, dest));
}

void Chip8Engine_CodeEmitter_x86::ADD_RtoR_32(X86Register dest, X86Register source)
{
	cache->write8(0x01);
	cache->write8(ModRegRM(3, source, dest));
}

void Chip8Engine_CodeEmitter_x86::ADD_ImmtoR_8(X86Register dest, uint8_t immediate)
{
	cache->write8(0x80);
	cache->write8(ModRegRM(3, (X86Register)0, dest));
	cache->write8(immediate);
}

void Chip8Engine_CodeEmitter_x86::ADD_ImmtoR_32(X86Register dest, uint32_t immediate)
{
	cache->write8(0x81);
	cache->write8(ModRegRM(3, (X86Register)0, dest));
	cache->write32(immediate);
}

void Chip8Engine_CodeEmitter_x86::ADD_ImmtoM_8(uint8_t* dest, uint8_t immediate)
{
	cache->write8(0x80);
	cache->write8(ModRegRM(0, (X86Register)0, (X86Register)MODREGRM_RM_DISP32));
	cache->write32((uint32_t)dest);
	cache->write8(immediate);
}

void Chip8Engine_CodeEmitter_x86::ADD_ImmtoM_32(uint32_t* dest, uint32_t immediate)
{
	cache->write8(0x81);
	cache->write8(ModRegRM(0, (X86Register)0, (X86Register)MODREGRM_RM_DISP32));
	cache->write32((uint32_t)dest);
	cache->write32(immediate);
}

void Chip8Engine_CodeEmitter_x86::ADD_RtoM_16(uint16_t * dest, X86Register source)
{
	cache->write8(0x66);
	cache->write8(0x01);
	cache->write8(ModRegRM(0, source, (X86Register)MODREGRM_RM_DISP32));
	cache->write32((uint32_t)dest);
}

void Chip8Engine_CodeEmitter_x86::ADD_MtoR_16(X86Register dest, uint16_t * source)
{
	cache->write8(0x66);
	cache->write8(0x03);
	cache->write8(ModRegRM(0, dest, (X86Register)MODREGRM_RM_DISP32));
	cache->write32((uint32_t)source);
}

void Chip8Engine_CodeEmitter_x86::ADD_RtoR_16(X86Register dest, X86Register source)
{
	cache->write8(0x66);
	cache->write8(0x01);
	cache->write8(ModRegRM(3, source, dest));
}

void Chip8Engine_CodeEmitter_x86::ADD_ImmtoR_16(X86Register dest, uint16_t immediate)
{
	cache->write8(0x66);
	cache->write8(0x81);
	cache->write8(ModRegRM(3, (X86Register)0, dest));
	cache->write16(immediate);
}

void Chip8Engine_CodeEmitter_x86::ADD_ImmtoM_16(uint16_t * dest, uint16_t immediate)
{
	cache->write8(0x66);
	cache->write8(0x81);
	cache->write8(ModRegRM(0, (X86Register)0, (X86Register)MODREGRM_RM_DISP32));
	cache->write32((uint32_t)dest);
	cache->write32(immediate);
}