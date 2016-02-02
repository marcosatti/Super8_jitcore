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

int Chip8Engine_Timers::runThread_Timers(void * data)
{
	// Follows SDL_CreateThread format.
	// Cast data to the timers class.
	Chip8Engine_Timers * timer_data = (Chip8Engine_Timers *) data;

	// Timers decrease @ 60 Hz until 0 is reached.
	// Sound timer will emit a beep noise until 0 is reached (ie while > 0).

	// For now, the atomic lock is implemented in asm. (spinlock)

	while (1) {
		SDL_Delay(16); // ~ 60Hz = 16ms sleeps.
		timer_data->TIMERS_SPIN_LOCK();
#ifdef USE_DEBUG
		char buffer[1000];
		sprintf_s(buffer, 1000, "delay_timer = %d, sound_timer = %d", timer_data->getDelayTimer(), timer_data->getSoundTimer());
		timer_data->logMessage(LOGLEVEL::L_DEBUG, buffer);
#endif
		timer_data->handleTimers();
		timer_data->TIMERS_SPIN_UNLOCK();
	}
	return 0;
}

void Chip8Engine_Timers::TIMERS_SPIN_LOCK()
{
	// Impl comes from https://en.wikipedia.org/wiki/Spinlock
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
	// Impl comes from https://en.wikipedia.org/wiki/Spinlock
	uint8_t * addr = &TIMERS_LOCK;
	__asm {
		mov edx, addr
		mov al, 0
		xchg al, [edx] // done atomically
	}
}

void Chip8Engine_Timers::handleTimers()
{
	if (delay_timer > 0)
	{
		delay_timer--;
	}
	if (sound_timer > 0)
	{
		sound_timer--;
#ifdef USE_VERBOSE
		logMessage(LOGLEVEL::L_INFO, "BEEP!");
#endif		
	}
}

uint8_t Chip8Engine_Timers::getDelayTimer()
{
	return delay_timer;
}

uint8_t Chip8Engine_Timers::getSoundTimer()
{
	return sound_timer;
}

void Chip8Engine_Timers::setDelayTimer(uint8_t value)
{
	delay_timer = value;
}

void Chip8Engine_Timers::setSoundTimer(uint8_t value)
{
	sound_timer = value;
}
