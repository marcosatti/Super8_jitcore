#pragma once

#include <cstdint>
#include "Chip8Globals.h"

#define MODREGRM_RM_DISP32 5
#define MODREGRM_RM_SIB 4

typedef enum {
	al = 0, ax = 0, eax = 0,
	cl = 1, cx = 1, ecx = 1,
	dl = 2, dx = 2, edx = 2,
	bl = 3, bx = 3, ebx = 3,
	ah = 4, sp = 4, esp = 4,
	ch = 5, bp = 5, ebp = 5,
	dh = 6, si = 6, esi = 6,
	bh = 7, di = 7, edi = 7
} X86Register;

class Chip8Engine_CodeEmitter_x86 {
public:
	Chip8Engine_CodeEmitter_x86();
	~Chip8Engine_CodeEmitter_x86();

	// DYNAREC HELPER FUNCTIONS
	void DYNAREC_EMIT_INTERRUPT(Chip8Globals::X86_STATE::X86_STATUS_CODE code, uint16_t c8_opcode);
	void DYNAREC_EMIT_INTERRUPT(Chip8Globals::X86_STATE::X86_STATUS_CODE code, uint16_t c8_opcode, uint16_t c8_return_pc);
	void DYNAREC_EMIT_MOV_EAX_EIP();
	void DYNAREC_EMIT_RETURN_CDECL_JUMP();

	// ADD opcode functions
	void ADD_RtoM_8(uint8_t* dest, X86Register source);
	void ADD_RtoM_32(uint32_t* dest, X86Register source);
	void ADD_MtoR_8(X86Register dest, uint8_t* source);
	void ADD_MtoR_32(X86Register dest, uint32_t* source);
	void ADD_RtoR_8(X86Register dest, X86Register source);
	void ADD_RtoR_32(X86Register dest, X86Register source);
	void ADD_ImmtoR_8(X86Register dest, uint8_t immediate);
	void ADD_ImmtoR_32(X86Register dest, uint32_t immediate);
	void ADD_ImmtoM_8(uint8_t* dest, uint8_t immediate);
	void ADD_ImmtoM_32(uint32_t* dest, uint32_t immediate);
	// 16 BIT (prefix 0x66)
	void ADD_RtoM_16(uint16_t* dest, X86Register source);
	void ADD_MtoR_16(X86Register dest, uint16_t* source);
	void ADD_RtoR_16(X86Register dest, X86Register source);
	void ADD_ImmtoR_16(X86Register dest, uint16_t immediate);
	void ADD_ImmtoM_16(uint16_t* dest, uint16_t immediate);

	// MOV opcode functions
	void MOV_RtoM_8(uint8_t* dest, X86Register source);
	void MOV_RtoM_32(uint32_t* dest, X86Register source);
	void MOV_MtoR_8(X86Register dest, uint8_t* source);
	void MOV_MtoR_32(X86Register dest, uint32_t* source);
	void MOV_PTRtoR_8(X86Register dest, X86Register PTR_source);
	void MOV_RtoPTR_8(X86Register PTR_dest, X86Register source);
	void MOV_RtoR_8(X86Register dest, X86Register source);
	void MOV_RtoR_32(X86Register dest, X86Register source);
	void MOV_ImmtoR_8(X86Register dest, uint8_t immediate);
	void MOV_ImmtoR_32(X86Register dest, uint32_t immediate);
	void MOV_ImmtoM_8(uint8_t* dest, uint8_t immediate);
	void MOV_ImmtoM_32(uint32_t* dest, uint32_t immediate);
	// 16 BIT (prefix 0x66)
	void MOV_RtoM_16(uint16_t* dest, X86Register source);
	void MOV_MtoR_16(X86Register dest, uint16_t* source);
	void MOV_RtoR_16(X86Register dest, X86Register source);
	void MOV_ImmtoR_16(X86Register dest, uint16_t immediate);
	void MOV_ImmtoM_16(uint16_t* dest, uint16_t immediate);

	// SUB opcode functions
	void SUB_RfromM_8(uint8_t* dest, X86Register source);
	void SUB_RfromM_32(uint32_t* dest, X86Register source);
	void SUB_MfromR_8(X86Register dest, uint8_t* source);
	void SUB_MfromR_32(X86Register dest, uint32_t* source);
	void SUB_RfromR_8(X86Register dest, X86Register source);
	void SUB_RfromR_32(X86Register dest, X86Register source);
	void SUB_ImmfromR_8(X86Register dest, uint8_t immediate);
	void SUB_ImmfromR_32(X86Register dest, uint32_t immediate);
	void SUB_ImmfromM_8(uint8_t* dest, uint8_t immediate);
	void SUB_ImmfromM_32(uint32_t* dest, uint32_t immediate);
	// 16 BIT (prefix 0x66)
	void SUB_RfromM_16(uint16_t* to, X86Register source);
	void SUB_MfromR_16(X86Register to, uint16_t* source);
	void SUB_RfromR_16(X86Register dest, X86Register source);
	void SUB_ImmfromR_16(X86Register to, uint16_t immediate);
	void SUB_ImmfromM_16(uint16_t* to, uint16_t immediate);

	// Bitwise opcode functions
	void OR_RwithR_8(X86Register dest, X86Register source);
	void OR_RwithM_8(X86Register dest, uint8_t* source);
	void OR_RwithR_32(X86Register dest, X86Register source);
	void AND_RwithR_8(X86Register dest, X86Register source);
	void AND_RwithM_8(X86Register dest, uint8_t* source);
	void AND_RwithR_32(X86Register dest, X86Register source);
	void AND_RwithImm_8(X86Register dest, uint8_t immediate);
	void XOR_RwithR_8(X86Register dest, X86Register source);
	void XOR_RwithM_8(X86Register dest, uint8_t* source);
	void XOR_RwithR_32(X86Register dest, X86Register source);
	void SHL_R_8(X86Register reg, uint8_t count);
	void SHR_R_8(X86Register reg, uint8_t count);
	void SHR_R_32(X86Register reg, uint8_t count);
	void CMP_RwithR_8(X86Register dest, X86Register source);
	void CMP_RwithImm_8(X86Register dest, uint8_t immediate);
	void CMP_RwithImm_32(X86Register dest, uint32_t immediate);

	// Jump opcode functions
	void JMP_REL_32(int32_t relative);
	void JMP_M_PTR_32(uint32_t * address);
	void JC_8(int8_t relative);
	void JNC_8(int8_t relative);
	void JE_8(int8_t relative);
	void JNE_8(int8_t relative);
	void JNG_8(int8_t relative);

	// Misc opcode functions
	void MUL_RwithR_8(X86Register source);
	void DIV_RwithR_8(X86Register source);
	void CALL_M_PTR_32(uint32_t * ptr_address); // CALL opcode
	void RET(); // RET opcode
	void POP(X86Register reg); // POP opcode
	void PUSH(X86Register reg); // PUSH opcode
	void RDTSC(); // Read time-stamp counter into EDX:EAX (used for random numbers)
private:
	// Helper function for ModRegRM byte of opcodes (TODO: make inline)
	uint8_t ModRegRM(uint8_t mod, X86Register reg, X86Register rm);

	// Helper function for SIB (scale-index-base) byte of opcodes (TODO: make inline)
	uint8_t SclIdxBase(uint8_t scale, X86Register index, X86Register base);
};
