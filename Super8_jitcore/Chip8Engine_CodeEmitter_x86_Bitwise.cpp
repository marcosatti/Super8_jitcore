#include "stdafx.h"

#include "Chip8Engine_CodeEmitter_x86.h"

using namespace Chip8Globals;

void Chip8Engine_CodeEmitter_x86::OR_RwithR_8(X86Register dest, X86Register source)
{
	cache->write8(0x08);
	cache->write8(ModRegRM(3, source, dest));
}

void Chip8Engine_CodeEmitter_x86::OR_RwithM_8(X86Register dest, uint8_t* source)
{
	cache->write8(0x0A);
	cache->write8(ModRegRM(0, dest, (X86Register)MODREGRM_RM_DISP32));
	cache->write32((uint32_t)source);
}

void Chip8Engine_CodeEmitter_x86::OR_RwithR_32(X86Register dest, X86Register source)
{
	cache->write8(0x09);
	cache->write8(ModRegRM(3, source, dest));
}

void Chip8Engine_CodeEmitter_x86::AND_RwithR_8(X86Register dest, X86Register source)
{
	cache->write8(0x20);
	cache->write8(ModRegRM(3, source, dest));
}

void Chip8Engine_CodeEmitter_x86::AND_RwithM_8(X86Register dest, uint8_t * source)
{
	cache->write8(0x22);
	cache->write8(ModRegRM(0, dest, (X86Register)MODREGRM_RM_DISP32));
	cache->write32((uint32_t)source);
}

void Chip8Engine_CodeEmitter_x86::AND_RwithR_32(X86Register dest, X86Register source)
{
	cache->write8(0x21);
	cache->write8(ModRegRM(3, source, dest));
}

void Chip8Engine_CodeEmitter_x86::AND_RwithImm_8(X86Register dest, uint8_t immediate)
{
	cache->write8(0x80);
	cache->write8(ModRegRM(3, (X86Register)4, dest));
	cache->write8(immediate);
}

void Chip8Engine_CodeEmitter_x86::XOR_RwithR_8(X86Register dest, X86Register source)
{
	cache->write8(0x30);
	cache->write8(ModRegRM(3, source, dest));
}

void Chip8Engine_CodeEmitter_x86::XOR_RwithM_8(X86Register dest, uint8_t * source)
{
	cache->write8(0x32);
	cache->write8(ModRegRM(0, dest, (X86Register)MODREGRM_RM_DISP32));
	cache->write32((uint32_t)source);
}

void Chip8Engine_CodeEmitter_x86::XOR_RwithR_32(X86Register dest, X86Register source)
{
	cache->write8(0x31);
	cache->write8(ModRegRM(3, source, dest));
}

void Chip8Engine_CodeEmitter_x86::SHL_R_8(X86Register reg, uint8_t count)
{
	cache->write8(0xC0);
	cache->write8(ModRegRM(3, (X86Register)4, reg));
	cache->write8(count);
}

void Chip8Engine_CodeEmitter_x86::SHR_R_8(X86Register reg, uint8_t count)
{
	cache->write8(0xC0);
	cache->write8(ModRegRM(3, (X86Register)5, reg));
	cache->write8(count);
}

void Chip8Engine_CodeEmitter_x86::SHR_R_32(X86Register reg, uint8_t count)
{
	cache->write8(0xC1);
	cache->write8(ModRegRM(3, (X86Register)5, reg));
	cache->write8(count);
}

void Chip8Engine_CodeEmitter_x86::CMP_RwithR_8(X86Register dest, X86Register source)
{
	cache->write8(0x38);
	cache->write8(ModRegRM(3, source, dest));
}

void Chip8Engine_CodeEmitter_x86::CMP_RwithImm_8(X86Register dest, uint8_t immediate)
{
	cache->write8(0x80);
	cache->write8(ModRegRM(3, (X86Register)7, dest));
	cache->write8(immediate);
}

void Chip8Engine_CodeEmitter_x86::CMP_RwithImm_32(X86Register dest, uint32_t immediate)
{
	cache->write8(0x81);
	cache->write8(ModRegRM(3, (X86Register)7, dest));
	cache->write32(immediate);
}
