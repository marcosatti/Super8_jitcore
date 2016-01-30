#include "stdafx.h"

#include <SDL.h>

#include "Headers\Globals.h"

#include "Headers\Chip8Globals\Chip8Globals.h"
#include "Headers\Chip8Engine\Chip8Engine_Interpreter.h"

// This file provides easier opcode management rather than having it all in the main engine.cpp file.
// TODO Implement: 0x0NNN (needed ?)

using namespace Chip8Globals;

Chip8Engine_Interpreter::Chip8Engine_Interpreter()
{
	// Register this component in logger
	logger->registerComponent(this);
}

Chip8Engine_Interpreter::~Chip8Engine_Interpreter()
{
	// Deregister this component in logger
	logger->deregisterComponent(this);
}

std::string Chip8Engine_Interpreter::getComponentName()
{
	return std::string("Interpreter");
}

void Chip8Engine_Interpreter::setOpcode(uint16_t c8_opcode)
{
	opcode = c8_opcode;
}

void Chip8Engine_Interpreter::emulateCycle() {
	// Decode Opcode
	// Initially work out what type of opcode it is by AND with 0xF000 and branch from that (looks at MSB)
	switch (opcode & 0xF000) {
	case 0x0000:
		handleOpcodeMSN_0();
		break;
	case 0x1000:
		handleOpcodeMSN_1();
		break;
	case 0x2000:
		handleOpcodeMSN_2();
		break;
	case 0x3000:
		handleOpcodeMSN_3();
		break;
	case 0x4000:
		handleOpcodeMSN_4();
		break;
	case 0x5000:
		handleOpcodeMSN_5();
		break;
	case 0x6000:
		handleOpcodeMSN_6();
		break;
	case 0x7000:
		handleOpcodeMSN_7();
		break;
	case 0x8000:
		handleOpcodeMSN_8();
		break;
	case 0x9000:
		handleOpcodeMSN_9();
		break;
	case 0xA000:
		handleOpcodeMSN_A();
		break;
	case 0xB000:
		handleOpcodeMSN_B();
		break;
	case 0xC000:
		handleOpcodeMSN_C();
		break;
	case 0xD000:
		handleOpcodeMSN_D();
		break;
	case 0xE000:
		handleOpcodeMSN_E();
		break;
	case 0xF000:
		handleOpcodeMSN_F();
		break;
	default:
		// Unknown opcode encountered
		char buffer[1000];
		_snprintf_s(buffer, 1000, "Unknown Opcode detected!");
		logMessage(LOGLEVEL::L_WARNING, buffer);
		break;
	}
}

void Chip8Engine_Interpreter::handleOpcodeMSN_0() {
	switch (opcode) {
	case 0x00E0:
	{
		// 0x00E0: Clears the screen
		// TODO: Check if correct.
#ifdef USE_SDL_GRAPHICS
		SDL_LockTexture(SDL_texture, NULL, (void**)&SDL_gfxmem, &SDL_pitch);
		memset(SDL_gfxmem, 0x00, 64 * 32 * sizeof(uint32_t));
		SDL_UnlockTexture(SDL_texture);
#else
		C8_STATE::C8_clearGFXMem();
#endif
		// V[0xF] = 0; // Need to set VF to 0?
		setDrawFlag(true);
		break;
	}
	case 0x00EE:
	{
		// 0x00EE: Returns from a subroutine
		// TODO: Check if correct.
		//cpu.pc = stack->getTopStack(); // Returns the previous stack address.
		//C8_incrementPC(); // Still need to advance stack by 2 as the stack returns the 'current' address.
		break;
	}
	default:
	{
		// 0x0NNN: Calls RCA 1802 program at address 0xNNN. (?)
		// TODO: Implement?. Skips instruction for now.
		break;
	}
	}
}

void Chip8Engine_Interpreter::handleOpcodeMSN_1() {
	// Only one subtype of opcode in this branch
	// 0x1NNN jumps to address 0xNNN (set PC)
	// TODO: check if correct
}

void Chip8Engine_Interpreter::handleOpcodeMSN_2() {
	// Only one subtype of opcode in this branch
	// 0x2NNN calls the subroutine at address 0xNNN
	//stack->setTopStack(cpu.pc); // Dont want to lose the current PC, so store it on the stack
	//cpu.pc = opcode & 0x0FFF; // Get the memory address within the opcode by AND
}

void Chip8Engine_Interpreter::handleOpcodeMSN_3() {
	// Only one subtype of opcode in this branch
	// 0x3XNN skips next instruction if VX equals NN
	// TODO: check if correct
	//uint8_t vx = (opcode & 0x0F00) >> 8;
	//uint8_t num = (opcode & 0x00FF);
	//if (cpu.V[vx] == num) C8_incrementPC(4);
	//else C8_incrementPC(); // Update PC by 2 bytes
}

void Chip8Engine_Interpreter::handleOpcodeMSN_4() {
	// Only one subtype of opcode in this branch
	// 0x4XNN skips next instruction if VX does not equal NN
	//// TODO: check if correct
	//uint8_t vx = (opcode & 0x0F00) >> 8;
	//uint8_t num = (opcode & 0x00FF);
	//if (cpu.V[vx] != num) C8_incrementPC(4);
	//else C8_incrementPC(); // Update PC by 2 bytes
}

void Chip8Engine_Interpreter::handleOpcodeMSN_5() {
	// Only one subtype of opcode in this branch
	// 0x5XY0 skips next instruction if VX equals XY
	//// TODO: check if correct
	//uint8_t vx = (opcode & 0x0F00) >> 8;
	//uint8_t vy = (opcode & 0x00F0) >> 4;
	//if (cpu.V[vx] == cpu.V[vy]) C8_incrementPC(4);
	//else C8_incrementPC(); // Update PC by 2 bytes
}

void Chip8Engine_Interpreter::handleOpcodeMSN_6() {
	// Only one subtype of opcode in this branch
	// 0x6XNN sets VX to NN
	//// TODO: check if correct
	//uint8_t vx = (opcode & 0x0F00) >> 8;
	//uint8_t num = (opcode & 0x00FF);
	//cpu.V[vx] = num;
	//C8_incrementPC(); // Update PC by 2 bytes
}

void Chip8Engine_Interpreter::handleOpcodeMSN_7() {
	// Only one subtype of opcode in this branch
	// 0x7XNN adds NN to Vx
	//// TODO: check if correct
	//uint8_t vx = (opcode & 0x0F00) >> 8;
	//uint8_t num = (opcode & 0x00FF);
	//cpu.V[vx] = cpu.V[vx] + num;
	//C8_incrementPC(); // Update PC by 2 bytes
}

void Chip8Engine_Interpreter::handleOpcodeMSN_8() {
	switch (opcode & 0x000F) {
	case 0x0000:
	{
		// 0x8XY0: Sets VX to the value of VY
		//// TODO: Check if correct
		//uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 16 to get to a single base16 digit.
		//uint8_t vy = (opcode & 0x00F0) >> 4; // Need to bit shift by 8 to get to a single base16 digit.
		//cpu.V[vx] = cpu.V[vy]; // Set Vx to Vy
		//C8_incrementPC(); // Goto next opcode
		break;
	}
	case 0x0001:
	{
		// 0x8XY1: Sets VX to VX OR VY
		//// TODO: Check if correct
		//uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 16 to get to a single base16 digit.
		//uint8_t vy = (opcode & 0x00F0) >> 4; // Need to bit shift by 8 to get to a single base16 digit.
		//cpu.V[vx] = cpu.V[vx] | cpu.V[vy]; // Set Vx to (Vx | Vy)
		//C8_incrementPC(); // Goto next opcode
		break;
	}
	case 0x0002:
	{
		// 0x8XY2: Sets VX to VX AND VY
		//// TODO: Check if correct
		//uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 16 to get to a single base16 digit.
		//uint8_t vy = (opcode & 0x00F0) >> 4; // Need to bit shift by 8 to get to a single base16 digit.
		//cpu.V[vx] = cpu.V[vx] & cpu.V[vy]; // Set Vx to (Vx & Vy)
		//C8_incrementPC(); // Goto next opcode
		break;
	}
	case 0x0003:
	{
		// 0x8XY3: Sets VX to VX XOR VY
		//// TODO: Check if correct
		//uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 16 to get to a single base16 digit.
		//uint8_t vy = (opcode & 0x00F0) >> 4; // Need to bit shift by 8 to get to a single base16 digit.
		//cpu.V[vx] = cpu.V[vx] ^ cpu.V[vy]; // Set Vx to (Vx ^ Vy)
		//C8_incrementPC(); // Goto next opcode
		break;
	}
	case 0x0004:
	{
		//// 0x8XY4: Adds Vy to Vx, setting VF to 1 when there is a carry and 0 when theres not.
		//uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 16 to get to a single base16 digit.
		//uint8_t vy = (opcode & 0x00F0) >> 4; // Need to bit shift by 8 to get to a single base16 digit.
		//if (cpu.V[vy] > (0xFF - cpu.V[vx])) cpu.V[0xF] = 1; // If Vy is larger than 255 - Vx, then the result will overflow and the carry flag will be set to 1.
		//else cpu.V[0xF] = 0; // The result does not overflow and carry flag is set to 0.
		//cpu.V[vx] = cpu.V[vx] + cpu.V[vy]; // Perform opcode
		//C8_incrementPC(); // Goto next opcode
		break;
	}
	case 0x0005:
	{
		// 0x8XY5: Vy is subtracted from Vx. VF set to 0 when theres a borrow, and 0 when there isnt.
		//// TODO: Check if correct
		//uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 16 to get to a single base16 digit.
		//uint8_t vy = (opcode & 0x00F0) >> 4; // Need to bit shift by 8 to get to a single base16 digit.
		//if (cpu.V[vy] > cpu.V[vx]) cpu.V[0xF] = 0; // If Vy is larger than Vx, then the result will underflow and the carry flag will be set to 0.
		//else cpu.V[0xF] = 1; // The result does not underflow and carry flag is set to 1.
		//cpu.V[vx] = cpu.V[vx] - cpu.V[vy]; // Perform opcode
		//C8_incrementPC(); // Goto next opcode
		break;
	}
	case 0x0006:
	{
		// 0x8XY6: Shifts Vx right by one. VF is set to the LSB of Vx before the shift.
		//// TODO: Check if correct
		//uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 16 to get to a single base16 digit.
		//cpu.V[0xF] = cpu.V[vx] & 0x01; // Set VF to LSB of Vx.
		//cpu.V[vx] = cpu.V[vx] >> 1; // Perform opcode
		//C8_incrementPC(); // Goto next opcode
		break;
	}
	case 0x0007:
	{
		// 0x8XY7: Sets Vx to Vy minus Vx. VF is set to 0 when theres a borrow, and 1 where there isnt.
		//// TODO: Check if correct
		//uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 16 to get to a single base16 digit.
		//uint8_t vy = (opcode & 0x00F0) >> 4; // Need to bit shift by 8 to get to a single base16 digit.
		//if (cpu.V[vx] > cpu.V[vy]) cpu.V[0xF] = 0; // If Vx is larger than Vy, then the result will underflow and the carry flag will be set to 0.
		//else cpu.V[0xF] = 1; // The result does not underflow and carry flag is set to 1.
		//cpu.V[vx] = cpu.V[vy] - cpu.V[vx]; // Perform opcode
		//C8_incrementPC(); // Goto next opcode
		break;
	}
	case 0x000E:
	{
		// 0x8XYE: Shifts Vx left by one. VF is set to the value of the MSB of Vx before the shift.
		//// TODO: Check if correct
		//uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 16 to get to a single base16 digit.
		//cpu.V[0xF] = (cpu.V[vx] & 0x80) >> 7; // Set VF to MSB of Vx. 0x80 = 0b10000000 and need to shift to the right by 7 places.
		//cpu.V[vx] = cpu.V[vx] << 1; // Perform opcode
		//C8_incrementPC(); // Goto next opcode
		break;
	}
	default:
	{
		char buffer[1000];
		_snprintf_s(buffer, 1000, "Unknown Opcode detected (in 0x8000)!");
		logMessage(LOGLEVEL::L_WARNING, buffer);
		//C8_incrementPC(); // Update PC by 2 bytes
		break;
	}
	}
}

void Chip8Engine_Interpreter::handleOpcodeMSN_9() {
	switch (opcode & 0x000F) {
	case 0x0000:
	{
		// 0x9XY0: Skips next instruction if register VX does not equal register VY
		//// TODO: Check if correct
		//uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 8 to get to a single base16 digit.
		//uint8_t vy = (opcode & 0x00F0) >> 4; // Need to bit shift by 4 to get to a single base16 digit.
		//if (cpu.V[vx] != cpu.V[vy]) C8_incrementPC(4); // Check if the two registers are not equal, and skip next instruction if true (skips next 2 bytes).
		//else C8_incrementPC(); // Else execute next instruction.
		break;
	}
	default:
	{
		char buffer[1000];
		_snprintf_s(buffer, 1000, "Unknown Opcode detected (in 0x9000)!");
		logMessage(LOGLEVEL::L_WARNING, buffer);
		//C8_incrementPC(); // Update PC by 2 bytes
		break;
	}
	}
}

void Chip8Engine_Interpreter::handleOpcodeMSN_A() {
	// Only one subtype of opcode in this branch
	// 0xANNN: Sets I to the address NNN
	//// TODO: Check if correct
	//cpu.I = (opcode & 0x0FFF); // Set I to address NNN
	//C8_incrementPC(); // Update PC by 2 bytes
}

void Chip8Engine_Interpreter::handleOpcodeMSN_B() {
	// Only one subtype of opcode in this branch
	// 0xBNNN: Sets PC to the address (NNN + V0)
	//// TODO: Check if correct
	//cpu.pc = ((opcode & 0x0FFF) + cpu.V[0x0]);
}

void Chip8Engine_Interpreter::handleOpcodeMSN_C() {
	// Only one subtype of opcode in this branch
	// 0xCXNN: Sets Vx to the result of 0xNN & (random number)
	// TODO: Check if correct.
	//uint8_t randnum = rand() % 256; // Get random number from 0 -> 255.
	//uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 8 to get to a single base16 digit.
	//uint8_t opcodenum = opcode & 0x0FF; // Number from opcode.
	//C8_STATE::cpu.V[vx] = opcodenum & randnum; // Set Vx to number from opcode AND random number.
}

void Chip8Engine_Interpreter::handleOpcodeMSN_D() {
	// Only one subtype of opcode in this branch
	/* 0xDXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
				Each row of 8 pixels is read as bit-coded starting from memory location I;
				I value doesn’t change after the execution of this instruction.
				As described above, VF is set to 1 if any screen pixels are flipped from
				set to unset when the sprite is drawn, and to 0 if that doesn’t happen */
				// TODO: check if correct.
	uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 8 to get to a single base16 digit.
	uint8_t vy = (opcode & 0x00F0) >> 4; // Need to bit shift by 4 to get to a single base16 digit.

	uint8_t xpixel = C8_STATE::cpu.V[vx]; // Get x-pixel position from register Vx
	uint8_t ypixel = C8_STATE::cpu.V[vy]; // Get y-pixel position from register Vy
	uint8_t nrows = (opcode & 0x000F); // Get num of rows to draw.
	uint32_t gfxarraypos = 0x0; // Variable used to calculate position within gfx memory array based on x and y positions.

	uint8_t newpixeldata = 0x0; // Variable used to hold the new pixel data from memory[I, I+1] etc.
	uint8_t oldpixelbit = 0x0; // Variable used to hold old pixel bit currently on screen.
	uint8_t newpixelbit = 0x0; // Variable used to hold new pixel bit, grabbed from the newpixeldata variable.

	C8_STATE::cpu.V[0xF] = 0; // Set VF to 0 initially (from specs).

#ifdef USE_SDL_GRAPHICS
	SDL_LockTexture(SDL_texture, NULL, (void**)&SDL_gfxmem, &SDL_pitch);
#endif
	for (int ypos = 0; ypos < nrows; ypos++) { // Loop through number of rows to display from opcode.
		newpixeldata = C8_STATE::memory[C8_STATE::cpu.I + ypos]; // Get the first row of pixel data.
		for (int xpos = 0; xpos < NUM_BITS_PER_BYTE; xpos++) { // Loop though the x-positions.
			gfxarraypos = ((ypixel + ypos) * GFX_XRES) + (xpixel + xpos); // Calculate the gfx memory array position
			newpixelbit = (newpixeldata & (0x80 >> xpos)); // Get the pixel bit value from within the 8-bit data. (will be > 0 if there is a pixel)
			if (newpixelbit != 0) { // Set new pixel only if there is data.
#ifdef USE_SDL_GRAPHICS
				// Apparently SDL is using 32bit pixels always even though its RGB888 (24)? Anyway, the higher order bits are unused (remember: little-endian on intel x86, number stored in memory back to front)
				(SDL_gfxmem[gfxarraypos] > 0) ? oldpixelbit = 1 : oldpixelbit = 0;
				if (oldpixelbit == 1) C8_STATE::cpu.V[0xF] = 1; // Get the previous pixel value (already in the form of 1 or 0). Set VF to 1 if ANY pixel will be unset (from specs, used for collision detection).
				SDL_gfxmem[gfxarraypos] ^= 0x00FFFFFF;
#else
				oldpixelbit = C8_STATE::gfxmem[gfxarraypos]; // Get the previous pixel value (already in the form of 1 or 0).
				if (oldpixelbit == 1) C8_STATE::cpu.V[0xF] = 1; // Set VF to 1 if ANY pixel will be unset (from specs, used for collision detection).
				C8_STATE::gfxmem[gfxarraypos] = C8_STATE::gfxmem[gfxarraypos] ^ 0x01; // Toggle pixel using XOR.
#endif
			}
		}
	}
#ifdef USE_SDL_GRAPHICS
	SDL_UnlockTexture(SDL_texture);
#endif
	setDrawFlag(true); // Set the draw flag to true.
}

void Chip8Engine_Interpreter::handleOpcodeMSN_E() {
	switch (opcode & 0x00FF) {
	case 0x009E:
	{
		// 0xEX9E: Skips the next instruction if the key stored in Vx is pressed.
		//// TODO: Check if correct.
		//uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 8 to get to a single base16 digit.
		//uint8_t keynum = cpu.V[vx]; // Get the key number from registry Vx.
		//if (key->getKeyState(keynum)) C8_incrementPC(4); // Skip next instruction if key is pressed.
		//else C8_incrementPC(); // Else goto next instruction.
		break;
	}
	case 0x00A1:
	{
		// 0xEXA1: Skips the next instruction if the key stored in Vx isnt pressed.
		//// TODO: Check if correct.
		//uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 8 to get to a single base16 digit.
		//uint8_t keynum = cpu.V[vx]; // Get the key number from registry Vx.
		//if (!key->getKeyState(keynum)) C8_incrementPC(4); // Skip next instruction if key is not pressed.
		//else C8_incrementPC(); // Else goto next instruction.
		break;
	}
	default:
	{
		char buffer[1000];
		_snprintf_s(buffer, 1000, "Unknown Opcode detected (in 0xE000)!");
		logMessage(LOGLEVEL::L_WARNING, buffer);
		//C8_incrementPC(); // Update PC by 2 bytes
		break;
	}
	}
}

void Chip8Engine_Interpreter::handleOpcodeMSN_F() {
	switch (opcode & 0x00FF) {
	case 0x0007:
	{
		// 0xFX07: Sets Vx to the value of the delay timer.
		//// TODO: check if correct.
		//uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 8 to get to a single base16 digit.
		//cpu.V[vx] = timers->getDelayTimer(); // Get delay timer and store it in Vx.
		//C8_incrementPC(); // Update PC by 2 bytes
		break;
	}
	case 0x000A:
	{
		// 0xFX0A: A key press is awaited, then stored in Vx.
		// TODO: Check if correct.
		//bool keypressed = false;
		//uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 8 to get to a single base16 digit.
		//for (int i = 0; i < NUM_KEYS; i++) {
		//	uint8_t keystate = key->getKeyState(i); // Get the keystate from the key object.
		//	if (keystate) {
		//		C8_STATE::cpu.V[vx] = i; // Set Vx to the key pressed (0x0 -> 0xF).
		//		keypressed = true;
		//	}
		//}
		//if (!keypressed) return; // Dont update PC if no key was pressed.
		//C8_incrementPC(); // Update PC by 2 bytes
		break;
	}
	case 0x0015:
	{
		// 0xFX15: Sets the delay timer to Vx.
		//// TODO: check if correct.
		//uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 8 to get to a single base16 digit.
		//timers->setDelayTimer(cpu.V[vx]);
		//C8_incrementPC(); // Update PC by 2 bytes
		break;
	}
	case 0x0018:
	{
		// 0xFX18: Sets the sound timer to Vx.
		//// TODO: check if correct.
		//uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 8 to get to a single base16 digit.
		//timers->setSoundTimer(cpu.V[vx]);
		//C8_incrementPC(); // Update PC by 2 bytes
		break;
	}
	case 0x001E:
	{
		// 0xFX1E: Adds Vx to I.
		//// TODO: check if correct.
		//uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 8 to get to a single base16 digit.
		//cpu.I = cpu.I + cpu.V[vx];
		//C8_incrementPC(); // Update PC by 2 bytes
		break;
	}
	case 0x0029:
	{
		// 0xFX29: Sets I to the location of the sprite for the character in Vx. Chars 0-F (in hex) are represented by a 4x5 font.
		//         ie: if V[x] = 0xA, set I to location of A in font sheet. Note that sprites are 8-bits wide, while fonts are 4-bits, so
		//             the 4-bits at the end are padded (0's).
		//// TODO: check if correct.
		//uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 8 to get to a single base16 digit.
		//cpu.I = cpu.V[vx] * FONT_WIDTH; // Set I to the location of the first byte of the font needed within the font set.
		//C8_incrementPC(); // Update PC by 2 bytes
		break;
	}
	case 0x0033:
	{
		// 0xFX33: Splits the decimal representation of Vx into 3 locations: hundreds stored in address I, tens in address I+1, and ones in I+2.
		//uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 8 to get to a single base16 digit.
		//C8_STATE::memory[C8_STATE::cpu.I] = C8_STATE::cpu.V[vx] / 100; // Hundreds go into address I
		//C8_STATE::memory[C8_STATE::cpu.I + 1] = (C8_STATE::cpu.V[vx] % 100) / 10; // Tens go into address I+1
		//C8_STATE::memory[C8_STATE::cpu.I + 2] = (C8_STATE::cpu.V[vx] % 100) % 10 /* / 1 */; // Ones go into address I+2
		break;
	}
	case 0x0055:
	{
		// 0xFX55: Copies all current values in registers V0 -> Vx to memory starting at address I.
		// TODO: check if correct.
		//uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 8 to get to a single base16 digit.
		//for (int i = 0x0; i <= vx; i++) {
		//	C8_STATE::memory[C8_STATE::cpu.I + i] = C8_STATE::cpu.V[i];
		//}
		break;
	}
	case 0x0065:
	{
		// 0xFX65: Copies memory starting from address I to all registers V0 -> Vx.
		// TODO: check if correct.
		//uint8_t vx = (opcode & 0x0F00) >> 8; // Need to bit shift by 8 to get to a single base16 digit.
		//for (int i = 0x0; i <= vx; i++) {
		//	C8_STATE::cpu.V[i] = C8_STATE::memory[C8_STATE::cpu.I + i];
		//}
		break;
	}
	default:
	{
		char buffer[1000];
		_snprintf_s(buffer, 1000, "Unknown Opcode detected (in 0xF000)!");
		logMessage(LOGLEVEL::L_WARNING, buffer);
		break;
	}
	}
}