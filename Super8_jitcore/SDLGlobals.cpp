#include "stdafx.h"
#include "SDLGlobals.h"

namespace SDLGlobals {
	const SDL_Color SDL_COLOR_WHITE = { 255,255,255,255 };
	const SDL_Color SDL_COLOR_BLACK = { 0,0,0,0 };

	SDL_Window * window;
	SDL_Renderer * renderer;
	SDL_Texture * texture;
	TTF_Font * font;
	SDL_Surface * fps_surf;
	SDL_Texture * fps_tex;
	SDL_Surface * cycle_surf;
	SDL_Texture * cycle_tex;
	SDL_Rect fps_render_location = { 0,0,0,0 };
	SDL_Rect cycle_render_location = { 0,0,0,0 };

	uint32_t * SDL_gfxmem; // Apparently SDL is using 32bit pixels always even though its RGB888 (24)? Anyway, the higher order bits are unused (remember: little-endian on intel x86, number stored in memory back to front)
	int pitch;

	uint64_t cycles = 0;
	uint64_t drawcycles = 0;
	uint64_t drawcycles_old = 0;
	uint32_t ticks = 0;
	uint32_t ticks_old = 0;

	void setupSDLGraphics()
	{
		if (SDL_Init(SDL_INIT_EVERYTHING) != 0) exit(1);
		// init window, renderer, memory and texture
		if ((window = SDL_CreateWindow("Super8_jitcore", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_WINDOW_SHOWN)) == NULL) exit(1);
		if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED)) == NULL) exit(1);
		if ((texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 64, 32)) == NULL) exit(1);
		if (TTF_Init() != 0) exit(1);
		if ((font = TTF_OpenFont("..\\fonts\\OpenSans-Regular.ttf", 18)) == NULL) exit(1);
	}
	void exitSDLGraphics()
	{
		SDL_DestroyTexture(texture);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}
}