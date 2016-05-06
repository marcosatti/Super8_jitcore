#pragma once

// Globals.h is used to define global macros etc.

// Global Logger for message formatting. External component to emulator.
#include "Headers\Logger\Logger.h"
#include "Headers\Logger\ILogComponent.h"

// The USE_SDL define. If set, SDL will be used to display graphics output. Sound output is not implemented yet.
#define USE_SDL

// Logging options. Top define will output the least information, while the bottom define will output the most information (and is very slow!).
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

// Used to log messages to the console. External component to emulator.
extern Logger * logger;