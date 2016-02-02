// Super8.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <cstdint>
#include <SDL.h>
#include <SDL_ttf.h>
#include <Windows.h>

#include "Headers\Globals.h"
#include "Headers\Chip8Globals\Chip8Globals.h"

#include "Headers\Chip8Engine\Chip8Engine.h"
#include "Headers\Chip8Engine\Chip8Engine_Key.h"

// Variables
const char * ROM_PATH = "..\\Chip8_Roms\\INVADERS";
#ifdef USE_SDL_GRAPHICS
const char * PROGRAM_TITLE = "Super8_jitcore";
const SDL_Color SDL_COLOR_LIGHT_GREY = { 180,180,180,0 };
const SDL_Color SDL_COLOR_BLACK = { 0,0,0,0 };
SDL_Window * window = NULL;
SDL_Renderer * renderer = NULL;
SDL_Texture * gfx_texture = NULL;
TTF_Font * font = NULL;
#endif

// NVIDIA optimus hack
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

// Function Declarations
void setupSDL();
void cleanupSDL();

int main(int argc, char **argv) {
	// Vars
	uint64_t c_cycles = 0;
	uint64_t c_cycles_old = 0;
	uint64_t c_drawcycles = 0;
	uint64_t c_drawcycles_old = 0;
	uint32_t c_ticks = 0;
	uint32_t c_ticks_old = 0;
#ifdef USE_SDL_GRAPHICS
	SDL_Surface * render_fps_surface = NULL;
	SDL_Texture * render_fps_texture = NULL;
	SDL_Surface * render_cycles_surface = NULL;
	SDL_Texture * render_cycles_texture = NULL;
	SDL_Rect render_fps_location = { 0,0,0,0 };
	SDL_Rect render_cycles_location = { 0,0,0,0 };
	char render_text_buffer[255];
#endif

	// Setup logging system.
	logger = new Logger(false);

	// Setup SDL system.
	setupSDL();

	// Setup Super8_jitcore emulator.
	Chip8Engine * super8 = new Chip8Engine();

	// Initialize the Chip8 system and load the game into the memory.
	super8->initialise();
	super8->loadProgram(ROM_PATH);

	// If SDL graphics are being used set the texture to be used
#ifdef USE_SDL_GRAPHICS
	Chip8Globals::SDL_texture = gfx_texture;
#endif

	// DEBUG: Set key state initially.
	Chip8Globals::key->clearKeyState();
	Chip8Globals::key->setKeyState(0x5, KEY_STATE::DOWN);
	Chip8Globals::key->setKeyState(0x4, KEY_STATE::DOWN);

	// Main program loop.
	SDL_Event sdlevent;
	bool quit = false;
	while (!quit) {
		// Handle SDL events.
		while (SDL_PollEvent(&sdlevent)) {
			if (sdlevent.type == SDL_QUIT) {
				quit = true;
			}
		}
		
		// Emulation Loop.
		super8->emulationLoop();

		// Render if draw flag is set.
		if (Chip8Globals::getDrawFlag()) {
			// Prepare Cycle and FPS count.
			c_ticks = SDL_GetTicks();
			if ((c_ticks - c_ticks_old) > 1000) {
#ifdef USE_SDL_GRAPHICS
				// FPS Count:
				if (render_fps_texture != NULL) SDL_DestroyTexture(render_fps_texture);
				sprintf_s(render_text_buffer, sizeof(render_text_buffer), "Drawcycles/s: %4.0f fps", (c_drawcycles - c_drawcycles_old) * 1000.0 / (c_ticks - c_ticks_old));
				render_fps_surface = TTF_RenderText_Blended(font, render_text_buffer, SDL_COLOR_LIGHT_GREY);
				render_fps_texture = SDL_CreateTextureFromSurface(renderer, render_fps_surface);
				SDL_QueryTexture(render_fps_texture, NULL, NULL, &render_fps_location.w, &render_fps_location.h);
				SDL_FreeSurface(render_fps_surface);

				// Cycle and Draw Count:
				if (render_cycles_texture != NULL) SDL_DestroyTexture(render_cycles_texture);
				sprintf_s(render_text_buffer, sizeof(render_text_buffer), "Cycle: %llu, Drawcycle: %llu", c_cycles, c_drawcycles);
				render_cycles_surface = TTF_RenderText_Blended(font, render_text_buffer, SDL_COLOR_LIGHT_GREY);
				render_cycles_texture = SDL_CreateTextureFromSurface(renderer, render_cycles_surface);
				SDL_QueryTexture(render_cycles_texture, NULL, NULL, &render_cycles_location.w, &render_cycles_location.h);
				render_cycles_location.y = render_fps_location.h;
				SDL_FreeSurface(render_cycles_surface);
#else
				// Print to console.
				printf("Cycle: %llu, Draw: %llu, Cycles/s: %4.0f, Drawcycles/s: %4.0f\n", c_cycles, c_drawcycles, (c_cycles - c_cycles_old) * 1000.0 / (c_ticks - c_ticks_old), (c_drawcycles - c_drawcycles_old) * 1000.0 / (c_ticks - c_ticks_old));
#endif

				// Update old cycle count.
				c_ticks_old = c_ticks;
				c_cycles_old = c_cycles;
				c_drawcycles_old = c_drawcycles;
			}

			// DEBUG: Change key states (randomly).
			if (c_ticks % 500 < 100) {
				Chip8Globals::key->setKeyState(0x4, (KEY_STATE)(Chip8Globals::key->getKeyState(0x4) ^ 1));
				Chip8Globals::key->setKeyState(0x6, (KEY_STATE)(Chip8Globals::key->getKeyState(0x6) ^ 1));
			}

			// Update draw cycles count and reset draw flag.
			c_drawcycles++;
			Chip8Globals::setDrawFlag(false);

			// Final render
			// There is a stutter that happens when rendering currently. This is due to how the games work, where they will 'spin' in a tight loop waiting for the delay timer to reach 0 (@ 60 Hz).
			// In this period where it is non-zero, no graphical updates will appear. However the emulator is working correctly, its just that there is nothing to update and show.
			// When the graphics/system timings are implemented properly (ie: refresh rate is set properly), this will be less apparent.
#ifdef USE_SDL_GRAPHICS
			SDL_RenderClear(renderer);
			if (gfx_texture != NULL) SDL_RenderCopy(renderer, gfx_texture, NULL, NULL);
			if (render_fps_texture != NULL) SDL_RenderCopy(renderer, render_fps_texture, NULL, &render_fps_location);
			if (render_cycles_texture != NULL) SDL_RenderCopy(renderer, render_cycles_texture, NULL, &render_cycles_location);
			SDL_RenderPresent(renderer);
#endif
		}

		// Update number of emulation cycles.
		c_cycles++;
	}

	// Cleanup SDL & emulator.
	delete super8;
	cleanupSDL();
	delete logger;

	return EXIT_SUCCESS;
}

void setupSDL() {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) exit(1);
#ifdef USE_SDL_GRAPHICS
	// Following is used if the graphics mode is used.
	// Initialise window, renderer, memory and texture.
	if ((window = SDL_CreateWindow(PROGRAM_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_WINDOW_SHOWN)) == NULL) exit(1);
	if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED)) == NULL) exit(1);
	if ((gfx_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32)) == NULL) exit(1);
	// Initialise font system.
	if (TTF_Init() != 0) exit(1);
	if ((font = TTF_OpenFont("..\\Fonts\\OpenSans-Regular.ttf", 18)) == NULL) exit(1);
#endif
}

void cleanupSDL() {
#ifdef USE_SDL_GRAPHICS
	SDL_DestroyTexture(gfx_texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
#endif
	SDL_Quit();
}