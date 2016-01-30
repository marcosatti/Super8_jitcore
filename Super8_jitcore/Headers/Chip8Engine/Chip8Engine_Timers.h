#pragma once

#include <cstdint>
#include <string>

#include "Headers\Globals.h"

class Chip8Engine_Timers : ILogComponent
{
public:
	Chip8Engine_Timers();
	~Chip8Engine_Timers();

	std::string getComponentName();

	// This component is run on a separate thread, so it has a lock variable, and functions associated with it.
	static int runThread_Timers(void * data); // Function is run on a separate SDL thread.
	void TIMERS_SPIN_LOCK(); // CALL BEFORE TIMER FUNCTIONS ARE CALLED.
	void TIMERS_SPIN_UNLOCK(); // CALL AFTER TIMER FUNCTIONS ARE CALLED.

	void handleTimers(); // USE WITH ABOVE FUNCTIONS.
	uint8_t getDelayTimer();
	uint8_t getSoundTimer();
	void setDelayTimer(uint8_t value);
	void setSoundTimer(uint8_t value);

private:
	uint8_t delay_timer; // A timer register that counts down to zero at 60Hz.
	uint8_t sound_timer; // A sound timer register that runs at 60Hz, and will emit a sound when it hits zero.
	uint8_t TIMERS_LOCK;
};
