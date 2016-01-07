#pragma once
#include <cstdint>
#include <SDL.h>
#include <SDL_ttf.h>

namespace SDLGlobals {
	extern SDL_Window * window;
	extern SDL_Renderer * renderer;
	extern SDL_Surface * surface;
	extern SDL_Texture * texture;
	extern uint32_t * SDL_gfxmem;
	extern uint64_t cycles;
	extern uint64_t drawcycles;
	extern uint64_t drawcycles_old;
	extern uint32_t ticks;
	extern uint32_t ticks_old;
	extern TTF_Font * font;
	extern SDL_Surface * fps_surf;
	extern SDL_Texture * fps_tex;
	extern SDL_Surface * cycle_surf;
	extern SDL_Texture * cycle_tex;
	extern const SDL_Color SDL_COLOR_WHITE;
	extern const SDL_Color SDL_COLOR_BLACK;
	extern SDL_Rect fps_render_location;
	extern SDL_Rect cycle_render_location;

	extern int pitch;

	extern void setupSDLGraphics();
	extern void exitSDLGraphics();
}