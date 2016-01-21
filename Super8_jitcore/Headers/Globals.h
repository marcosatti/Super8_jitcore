#pragma once

// Globals.h is used to define global macros etc.

// SDL
#define USE_SDL

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