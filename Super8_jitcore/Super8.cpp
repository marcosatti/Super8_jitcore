// Super8.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <cstdint>
#include <time.h>

#include "SDLGlobals.h"
#include "Chip8Engine.h"

// NVIDIA optimus hack
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

using namespace SDLGlobals;

int main(int argc, char **argv) {
	// Set up render system and register input callbacks
	SDLGlobals::setupSDLGraphics();
	//setupInput();

	// Setup chip8 jitcore emulator
	Chip8Engine * mChip8 = new Chip8Engine();

	// Initialize the Chip8 system and load the game into the memory
	mChip8->initialise();
	mChip8->loadProgram("..\\chip8roms\\PONG2");

	// Set keystate initially
	for (int i = 0; i < 16; i++) {
		Chip8Globals::key->key[i] = 0;
	}
	Chip8Globals::key->key[0x5] = 1;
	Chip8Globals::key->key[0x4] = 1;

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
		if (getDrawFlag()) {
			sdlgfxupdate = true;
			drawcycles++;
			setDrawFlag(false);
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

			// cahnge key state
			Chip8Globals::key->key[0x4] ^= 1;
			Chip8Globals::key->key[0x6] ^= 1;
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

	// deconstruct graphics & emulator
	SDLGlobals::exitSDLGraphics();
	delete mChip8;

	return 0;
}