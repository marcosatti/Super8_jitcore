#pragma once

#include <cstdint>
#include <string>

#include "Headers\Globals.h"

namespace Chip8Engine {

	class Timers : ILogComponent
	{
	public:
		uint8_t delay_timer; // A timer register that counts down to zero at 60Hz.
		uint8_t sound_timer; // A sound timer register that runs at 60Hz, and will emit a sound when it hits zero.

		std::string getComponentName();

		// Timers variables above are directly accessed by the x86 code; see the translator opcodes FX15, FX18 etc for how it works.

		Timers();
		~Timers();
	private:
	};

}