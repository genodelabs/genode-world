/*
 * \brief  Simple SDL test program
 * \author Stefan Kalkowski
 * \date   2008-12-12
 */

/*
 * Copyright (c) <2008> Stefan Kalkowski
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/* SDL includes */
#include <SDL2/SDL.h>

/* Genode includes */
#include <base/env.h>
#include <timer_session/connection.h>

static void draw(SDL_Surface * const screen, int w, int h, int v)
{
	if (screen == nullptr) { return; }

	/* paint something into pixel buffer */
	short* const pixels = (short*) screen->pixels;
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			pixels[i*w+j] = ((i+v)/32)*32*64 + ((j+v)/32)*32 + (i*j+v)/1024;
		}
	}
}

static SDL_Window *create_window(int w, int h)
{
	SDL_Window *window = SDL_CreateWindow("sdl2 test window",
	                                      SDL_WINDOWPOS_UNDEFINED,
	                                      SDL_WINDOWPOS_UNDEFINED,
	                                      w, h,
	                                      SDL_WINDOW_FULLSCREEN);
	if (window == nullptr) {
		printf("Error: could not create window: %s\n", SDL_GetError());
	}

	return window;
}

#if 0
static SDL_Surface *resize_screen(SDL_Surface * const screen, int w, int h)
{
	if (screen == nullptr) { return nullptr; }

	int oldw = screen->w;
	int oldh = screen->h;

	SDL_Surface *nscreen = set_video_mode(w, h);
	if (nscreen == nullptr) {
		printf("Error: could not resize %dx%d -> %dx%d: %s\n",
		       oldw, oldh, w, h, SDL_GetError());
		return nullptr;
	}

	return nscreen;
}
#endif

static void dump_supported_features()
{
	int const cpu_count = SDL_GetCPUCount();
	int const cache_l1 = SDL_GetCPUCacheLineSize();

	SDL_bool const has_rdtsc   = SDL_HasRDTSC();
	SDL_bool const has_altivec = SDL_HasAltiVec();
	SDL_bool const has_mmx     = SDL_HasMMX();
	SDL_bool const has_3dnow   = SDL_Has3DNow();
	SDL_bool const has_sse     = SDL_HasSSE();
	SDL_bool const has_sse2    = SDL_HasSSE2();
	SDL_bool const has_sse3    = SDL_HasSSE3();
	SDL_bool const has_sse41   = SDL_HasSSE41();
	SDL_bool const has_sse42   = SDL_HasSSE42();
	SDL_bool const has_avx     = SDL_HasAVX();
	SDL_bool const has_avx2    = SDL_HasAVX2();
	SDL_bool const has_avx512f = SDL_HasAVX512F();
	SDL_bool const has_armsimd = SDL_HasARMSIMD();
	SDL_bool const has_armneon = SDL_HasNEON();
	int const system_ram       = SDL_GetSystemRAM();
	size_t simd_alignment      = SDL_SIMDGetAlignment();

	Genode::log("CPU count : " , cpu_count,
	            ", L1 cache line size: ", cache_l1);
	Genode::log("System ram: ", system_ram,
	            ", SIMD alignment: ", simd_alignment);
	Genode::log("Features:", has_rdtsc ? " rdtsc" : "",
	                         has_altivec ? " altivec" : "",
	                         has_mmx ? " mmx" : "",
	                         has_3dnow ? " 3dnow" : "",
	                         has_sse ? " sse" : "",
	                         has_sse2 ? " sse2" : "",
	                         has_sse3 ? " sse3" : "",
	                         has_sse41 ? " sse41" : "",
	                         has_sse42 ? " sse42" : "",
	                         has_avx ? " avx" : "",
	                         has_avx2 ? " avx2" : "",
	                         has_avx512f ? " avx512f" : "",
	                         has_armsimd ? " ARM SIMD" : "",
	                         has_armneon ? " ARM NEON" : "");
}


extern "C" void wait_for_continue();


int main(int, char*[] )
{
//	wait_for_continue();

	if (SDL_Init(SDL_INIT_VIDEO) == -1) {
		printf("%u SDL error: %s\n", __LINE__, SDL_GetError());
		return 1;
	}

	dump_supported_features();

	SDL_Window * const window = create_window(0, 0);
	if (!window) {
		printf("%u SDL error: %s\n", __LINE__, SDL_GetError());
		return 1;
	}

	SDL_Surface *surface = SDL_GetWindowSurface(window);
	if (!surface) {
		printf("%u SDL error: %s\n", __LINE__, SDL_GetError());
		return 1;
	}

	/* test renderer setup */
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
	                                            SDL_RENDERER_SOFTWARE);
//	                                            SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		printf("%u SDL error: %s\n", __LINE__, SDL_GetError());
		return 1;
	}

	SDL_RendererInfo renderer_info;
	if (SDL_GetRendererInfo(renderer, &renderer_info)) {
		printf("%u SDL error: %s\n", __LINE__, SDL_GetError());
		return 1;
	}

	printf("renderer: %s\n", renderer_info.name);

	int content_width = 640;
	int content_height = 480;
	if (SDL_RenderSetLogicalSize(renderer, content_width, content_height)) {
		printf("%u SDL error: %s\n", __LINE__, SDL_GetError());
		return 1;
	}

	/* test some primitives */
	if (SDL_FillRect(surface, NULL,
	                 SDL_MapRGB( surface->format, 0xFF, 0xFF, 0xFF ))) {
		printf("%u SDL error: %s\n", __LINE__, SDL_GetError());
		return 1;
	}
            
	if (SDL_UpdateWindowSurface(window)) {
		printf("%u SDL error: %s\n", __LINE__, SDL_GetError());
		return 1;
	}

	SDL_Delay(1000);

	unsigned loop_cnt = 0;

	bool done = false;
	while (!done) {
		loop_cnt ++;

		draw(surface, surface->w, surface->h, loop_cnt*10);
		SDL_Delay(50);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch(event.type) {
			case SDL_KEYDOWN:
				printf("%s\n", SDL_GetKeyName(event.key.keysym.sym));
				done = true;
				break;
/*
			case SDL_VIDEORESIZE:
				screen = resize_screen(screen, event.resize.w, event.resize.h);
				if (screen == nullptr) { done = true; }

				break;
*/
			}
		}

		//Update the surface
		if (SDL_UpdateWindowSurface(window)) {
			printf("%u SDL error: %s\n", __LINE__, SDL_GetError());
			return 1;
		}
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
