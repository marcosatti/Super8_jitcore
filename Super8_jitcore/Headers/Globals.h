#pragma once

// Globals.h is used to define global macros etc.

// Global Logger
#include "Headers\Logger\Logger.h"
#include "Headers\Logger\ILogComponent.h"

// SDL
#define USE_SDL_GRAPHICS

// Emulation Accuracy Options
// There are references online that say the Chip8 runs at 500 Hz, which equates to 2ms per instruction.
// Attempts to delay emulation by ((uint)1000/TARGET_FRAMES_PER_SECOND - execution time since last draw call)ms.
#define LIMIT_SPEED_BY_DRAW_CALLS
// TODO: implement.
//#define LIMIT_SPEED_BY_INSTRUCTIONS
#if defined(LIMIT_SPEED_BY_DRAW_CALLS) || defined(LIMIT_SPEED_BY_INSTRUCTIONS)
#define LIMITER_ON
#define TARGET_FRAMES_PER_SECOND 60
#endif

// Logging
//#define USE_VERBOSE
//#define USE_DEBUG
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