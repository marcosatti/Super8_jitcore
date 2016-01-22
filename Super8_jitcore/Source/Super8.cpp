// Super8.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <cstdint>
#include <Windows.h>

#include "Headers\Globals.h"
#include "Headers\SDLGlobals.h"
#include "Headers\Chip8Globals\Chip8Globals.h"

#include "Headers\Super8.h"

#include "Headers\Chip8Engine\Chip8Engine.h"
#include "Headers\Chip8Engine\Chip8Engine_Key.h"

// NVIDIA optimus hack
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

using namespace SDLGlobals;

int main(int argc, char **argv) {
	// Setup logging system
	logger = new Logger(false);

	// Set up render system and register input callbacks
#ifdef USE_SDL
	SDLGlobals::setupSDLGraphics();
#endif
	//setupInput();

	// Setup chip8 jitcore emulator
	Chip8Engine * mChip8 = new Chip8Engine();

	// Initialize the Chip8 system and load the game into the memory
	mChip8->initialise();
	mChip8->loadProgram("..\\Chip8_Roms\\PONG2");

	// Set keystate initially
	Chip8Globals::key->clearKeyState();
	Chip8Globals::key->setKeyState(0x1, KEY_STATE::DOWN);

#ifdef USE_SDL
	// Emulate
	char fps_buffer[255];
	char cycle_buffer[255];
	bool sdlgfxupdate = false;
	SDL_Event e;
	bool quit = false;
	while (!quit) {
		// Handle events
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				quit = true;
			}
		}

		// Emulation loop
		mChip8->emulationLoop();

		// C8 Render
		if (Chip8Globals::getDrawFlag()) {
			sdlgfxupdate = true;
			drawcycles++;
			Chip8Globals::setDrawFlag(false);
		}

		// Print fps
		ticks = SDL_GetTicks();
		if ((ticks - ticks_old) > 1000) {
			if (fps_tex != NULL) SDL_DestroyTexture(fps_tex);
			sprintf_s(fps_buffer, 255, "FPS: %4.0f fps", (drawcycles - drawcycles_old) * 1000.0 / (ticks - ticks_old));
			fps_surf = TTF_RenderText_Blended(font, fps_buffer, SDL_COLOR_WHITE);
			fps_tex = SDL_CreateTextureFromSurface(renderer, fps_surf);
			SDL_QueryTexture(fps_tex, NULL, NULL, &fps_render_location.w, &fps_render_location.h);
			SDL_FreeSurface(fps_surf);
			sdlgfxupdate = true;
			drawcycles_old = drawcycles;
			ticks_old = ticks;

			// change key state
			Chip8Globals::key->setKeyState(0x1, (KEY_STATE)(Chip8Globals::key->getKeyState(0x1) ^ 1));
			Chip8Globals::key->setKeyState(0xC, (KEY_STATE)(Chip8Globals::key->getKeyState(0xC) ^ 1));
		}

		// Print cycles
		if (drawcycles % 30 == 0) {
			if (cycle_tex != NULL) SDL_DestroyTexture(cycle_tex);
			sprintf_s(cycle_buffer, 255, "Cycle: %llu, Draw: %llu", cycles, drawcycles);
			cycle_surf = TTF_RenderText_Blended(font, cycle_buffer, SDL_COLOR_WHITE);
			cycle_tex = SDL_CreateTextureFromSurface(renderer, cycle_surf);
			SDL_QueryTexture(cycle_tex, NULL, NULL, &cycle_render_location.w, &cycle_render_location.h);
			cycle_render_location.y = fps_render_location.h;
			SDL_FreeSurface(cycle_surf);
			sdlgfxupdate = true;
		}

		// Final render
		if (sdlgfxupdate) {
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderCopy(renderer, fps_tex, NULL, &fps_render_location);
			SDL_RenderCopy(renderer, cycle_tex, NULL, &cycle_render_location);
			SDL_RenderPresent(renderer);
			sdlgfxupdate = false;
		}
		cycles++;
	}
#else
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) exit(1);
	//printf("Performance test mode. No graphics or sound!\n");
	uint64_t cycles_old = 0;
	while (1) {
		// Emulation loop
		mChip8->emulationLoop();

		// C8 Render
		if (Chip8Globals::getDrawFlag()) {
			//mChip8->DEBUG_renderGFXText();
			drawcycles++;
			Chip8Globals::setDrawFlag(false);
		}

		ticks = SDL_GetTicks();
		if ((ticks - ticks_old) > 1000) {
<<<<<<< HEAD
			//printf("Super8:			Cycle: %llu, Cycles per second: %8.0f\n", cycles, (cycles - cycles_old) * 1000.0 / (ticks - ticks_old));
=======
			printf("Super8:			Cycle: %llu, Cycles per second: %8.0f\n", cycles, (cycles - cycles_old) * 1000.0 / (ticks - ticks_old));
>>>>>>> block_test_perf
			ticks_old = ticks;
			cycles_old = cycles;
		}
		cycles++;
	}
#endif

	// deconstruct graphics & emulator
#ifdef USE_SDL
	SDLGlobals::exitSDLGraphics();
#endif
	delete mChip8;
	delete logger;

	return 0;
}