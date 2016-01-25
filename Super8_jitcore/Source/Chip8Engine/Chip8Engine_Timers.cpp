#include "stdafx.h"
#include <sstream>

#include <SDL.h>

#include "Headers\Globals.h"

#include "Headers\Chip8Engine\Chip8Engine_Timers.h"

std::string Chip8Engine_Timers::getComponentName()
{
	return std::string("Timers");
}

Chip8Engine_Timers::Chip8Engine_Timers()
{
	// Register this component in logger
	logger->registerComponent(this);
	delay_timer = 0;
	sound_timer = 0;
	TIMERS_LOCK = 0;
}

Chip8Engine_Timers::~Chip8Engine_Timers()
{
	// Deregister this component in logger
	logger->deregisterComponent(this);
}

int Chip8Engine_Timers::Thread_handleTimers(void * data)
{
	// Follows SDL_CreateThread format.
	// Cast data to the timers class.
	Chip8Engine_Timers * timer_data = (Chip8Engine_Timers *) data;

	// Timers decrease @ 60 Hz until 0 is reached.
	// Sound timer will emit a beep noise until 0 is reached (ie while > 0).

	// For now, the atomic lock is implemented in asm. (spinlock)

	while (1) {
		SDL_Delay(17); // ~ 60Hz = 17ms sleeps.
		timer_data->TIMERS_SPIN_LOCK();
#ifdef USE_DEBUG
		char buffer[1000];
		_snprintf_s(buffer, 1000, "delay_timer = %d, sound_timer = %d", timer_data->delay_timer, timer_data->sound_timer);
		timer_data->logMessage(LOGLEVEL::L_DEBUG, buffer);
#endif
		if (timer_data->delay_timer > 0)
		{
			timer_data->delay_timer--;
		}
		if (timer_data->sound_timer > 0)
		{
			timer_data->sound_timer--;
			timer_data->logMessage(LOGLEVEL::L_INFO, "BEEP!");
		}
		timer_data->TIMERS_SPIN_UNLOCK();
	}
	return 0;
}

void Chip8Engine_Timers::TIMERS_SPIN_LOCK()
{
	uint8_t * addr = &TIMERS_LOCK;
	__asm {
	spin_lock:
		mov edx, addr
		mov al, 1
		xchg al, [edx] // done atomically
		test al, al
		jnz spin_lock
	}
}

void Chip8Engine_Timers::TIMERS_SPIN_UNLOCK()
{
	uint8_t * addr = &TIMERS_LOCK;
	__asm {
		mov edx, addr
		mov al, 0
		xchg al, [edx] // done atomically
	}
}
