#pragma once

// Globals.h is used to define global macros etc.

// Global Logger
#include "Headers\Logger\Logger.h"
#include "Headers\Logger\ILogComponent.h"

// SDL
#define USE_SDL_GRAPHICS

// Emulation Accuracy Options - use only 1 or none at all.
// There are references online that say the Chip8 runs at 500 Hz, which equates to 2ms per instruction (equates to 60-70 fps from testing with INVADERS).
// Limiting by draw calls seems to have a more stable experience than by instructions.
//
// Attempts to delay emulation by (1000/TARGET_FRAMES_PER_SECOND - execution time since last draw call)ms. Offers a balance of accuracy vs performance trade off. A target of 60 fps seems good for most roms.
#define LIMIT_SPEED_BY_DRAW_CALLS
//
// Attempts to delay emulation by accurately simulating the clock speed of the Chip8 cpu, by inserting (1000/TARGET_CPU_SPEED_HZ)ms delays between each instruction. 
//#define LIMIT_SPEED_BY_INSTRUCTIONS

#if defined(LIMIT_SPEED_BY_DRAW_CALLS) || defined(LIMIT_SPEED_BY_INSTRUCTIONS)
#define LIMITER_ON
#define TARGET_FRAMES_PER_SECOND 60
#define TARGET_CPU_SPEED_HZ 500
#endif

// Logging
//#define USE_VERBOSE
#define USE_DEBUG
//#define USE_DEBUG_EXTRA
#ifdef USE_DEBUG_EXTRA
#ifndef USE_DEBUG
#define USE_DEBUG
#endif
#endif
#ifdef USE_DEBUG
#ifndef USE_VERBOSE
#define USE_VERBOSE
#endif
#endif

extern Logger * logger;