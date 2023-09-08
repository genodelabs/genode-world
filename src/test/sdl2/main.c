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

#include <SDL2/SDL.h>
#include <stdio.h>


static void draw(SDL_Surface * const screen, int w, int h, int v)
{
	if (screen == NULL) { return; }

	/* paint something into pixel buffer */
	Uint32* const pixels = (Uint32*) screen->pixels;
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
	if (window == NULL) {
		printf("Error: could not create window: %s\n", SDL_GetError());
	}

	return window;
}


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

	printf("CPU count : %d L1 cache line size: %d\n",
	       cpu_count, cache_l1);
	printf("System ram: %d SIMD alignment: %zu\n",
	       system_ram, simd_alignment);
	printf("Features: ");
	if (has_rdtsc)   printf("rdtsc ");
	if (has_altivec) printf("altivec ");
	if (has_mmx)     printf("mmx ");
	if (has_3dnow)   printf("3dnow ");
	if (has_sse)     printf("sse ");
	if (has_sse2)    printf("sse2 ");
	if (has_sse3)    printf("sse3 ");
	if (has_sse41)   printf("sse41 ");
	if (has_sse42)   printf("sse42 ");
	if (has_avx)     printf("avx ");
	if (has_avx2)    printf("avx2 ");
	if (has_avx512f) printf("avx512f ");
	if (has_armsimd) printf("ARM SIMD ");
	if (has_armneon) printf("ARM NEON ");
	printf("\n");
}


int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

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

	SDL_bool done = SDL_FALSE;
	while (!done) {
		loop_cnt ++;

		draw(surface, surface->w, surface->h, loop_cnt*10);
		SDL_Delay(50);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch(event.type) {
			case SDL_KEYDOWN:
				printf("%s\n", SDL_GetKeyName(event.key.keysym.sym));
				done = SDL_TRUE;
				break;
			}
		}

		if (SDL_UpdateWindowSurface(window)) {
			printf("%u SDL error: %s\n", __LINE__, SDL_GetError());
			return 1;
		}
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
