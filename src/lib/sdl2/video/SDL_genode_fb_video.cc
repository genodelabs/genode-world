/*
 * \brief  Genode-specific video backend
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

/* Genode includes */
#include <base/attached_ram_dataspace.h>
#include <base/attached_rom_dataspace.h>
#include <base/env.h>
#include <base/log.h>
#include <gui_session/connection.h>

/* local includes */
#include <SDL_genode_internal.h>


extern Genode::Env     &global_env();
extern Gui::Connection &global_gui();

extern Genode::Mutex event_mutex;
extern Video         video_events;


extern "C" {

#include "../../SDL_internal.h"

#include <dlfcn.h>

#include <SDL.h>
#include <SDL_video.h>
#include <SDL_mouse.h>
#include <SDL_mouse_c.h>
#include "SDL_sysvideo.h"
#include "SDL_pixels_c.h"
#include "SDL_events_c.h"
#include "SDL_genode_fb_events.h"

#if defined(SDL_VIDEO_OPENGL_EGL)
#include <SDL_egl_c.h>

#define Genode_GLES_GetAttribute SDL_EGL_GetAttribute
#define Genode_GLES_GetProcAddress SDL_EGL_GetProcAddress
#define Genode_GLES_UnloadLibrary SDL_EGL_UnloadLibrary
#define Genode_GLES_SetSwapInterval SDL_EGL_SetSwapInterval
#define Genode_GLES_GetSwapInterval SDL_EGL_GetSwapInterval

int Genode_GLES_LoadLibrary(_THIS, const char *path);
SDL_GLContext Genode_GLES_CreateContext(_THIS, SDL_Window * window);
int Genode_GLES_SwapWindow(_THIS, SDL_Window * window);
int Genode_GLES_MakeCurrent(_THIS, SDL_Window * window, SDL_GLContext context);
void Genode_GLES_GetDrawableSize(_THIS, SDL_Window * window, int * w, int * h);
void Genode_GLES_DeleteContext(_THIS, SDL_GLContext context);
#endif

	struct Sdl_framebuffer
	{
		Genode::Env        &_env;
		Gui::Connection    &_gui;
		Gui::Top_level_view _view { _gui };

		Genode::Attached_rom_dataspace _config_rom { _env, "config" };

		Gui::Area _mode { };

		void _handle_mode_change()
		{
			Gui::Rect const gui_win = _gui.window().convert<Gui::Rect>(
				[&] (Gui::Rect rect) { return rect; },
				[&] (Gui::Undefined) { return _gui.panorama().convert<Gui::Rect>(
					[&] (Gui::Rect rect) { return rect; },
					[&] (Gui::Undefined) { return Gui::Rect { }; }); });

			if (!gui_win.valid())
				return;

			_mode = gui_win.area;
			{
				Genode::Mutex::Guard guard(event_mutex);
				video_events.resize_pending = true;
				video_events.width  = _mode.w;
				video_events.height = _mode.h;
			}
		}

		Genode::Io_signal_handler<Sdl_framebuffer> _mode_handler {
			_env.ep(), *this, &Sdl_framebuffer::_handle_mode_change };

		Sdl_framebuffer(Genode::Env &env, Gui::Connection &gui)
		:
			_env(env), _gui(gui)
		{
			_gui.info_sigh(_mode_handler);
			_view.front();

			for (;;) {
				if (_mode.valid())
					break;
				_env.ep().wait_and_dispatch_one_io_signal();
			}

			_config_rom.update();
			_config_rom.xml().with_optional_sub_node("initial",
				[&] (Genode::Xml_node const &initial) {
					_mode = Gui::Area::from_xml(initial); });
		}

		~Sdl_framebuffer()
		{
			/* clean up and reduce noise about invalid signals */
			_gui.info_sigh(Genode::Signal_context_capability());
			dataspace(0, 0);
		}

		/************************************
		 ** Framebuffer::Session Interface **
		 ************************************/

		Genode::Dataspace_capability dataspace(unsigned width, unsigned height)
		{
			_mode = { width, height };

			_gui.buffer({ .area = _mode, .alpha = false });

			_view.area({ width, height });

			return _gui.framebuffer.dataspace();
		}

		Gui::Area mode() const { return _mode; }

		void refresh(Framebuffer::Rect rect) { _gui.framebuffer.refresh(rect); }

		void title(char const *string)
		{
			_gui.enqueue<Gui::Session::Command::Title>(_view.id(), string);
			_gui.execute();
		}
	};

	static char const * const surface_name = "genode_surface";

	struct Genode_Driverdata
	{
		Genode::Constructible<Sdl_framebuffer>                framebuffer;
		Genode::Constructible<Genode::Attached_dataspace>     fb_mem;
		Genode::Constructible<Genode::Attached_ram_dataspace> fb_double;
		Gui::Area                                             scr_mode;

#if defined(SDL_VIDEO_OPENGL_EGL)
		Genode_egl_window egl_window;
		EGLSurface        egl_surface;
#endif
	};

	/****************************************
	 * Genode_Fb driver bootstrap functions *
	 ****************************************/

	static int GenodeVideo_Available(void)
	{
		return 1;
	}


	static void GenodeVideo_DeleteDevice(SDL_VideoDevice * const device)
	{
		if (!device || !device->driverdata)
			return;

		Genode_Driverdata &drv = *(Genode_Driverdata *)device->driverdata;

		if (drv.framebuffer.constructed())
			drv.framebuffer.destruct();
	}

	static int Genode_Fb_CreateWindowFramebuffer(SDL_VideoDevice * const device,
	                                             SDL_Window * const window,
	                                             Uint32 * format,
	                                             void ** pixels,
	                                             int *pitch)
	{
		if (!device || !window || !format || !pixels || !pitch || !device->driverdata)
			return SDL_SetError("invalid pointer");

		Genode_Driverdata &drv = *(Genode_Driverdata *)device->driverdata;

		Uint32 const surface_format = SDL_PIXELFORMAT_ARGB8888;

		/* Free the old surface */
		SDL_Surface *surface = (SDL_Surface *)SDL_GetWindowData(window,
		                                                        surface_name);
		if (surface) {
			SDL_SetWindowData(window, surface_name, NULL);
			SDL_FreeSurface(surface);
			surface = NULL;
		}

		/* get 32-bit RGB mask values */
		int bpp;
		Uint32 r_mask, g_mask, b_mask, a_mask;
		if (!SDL_PixelFormatEnumToMasks(surface_format, &bpp, &r_mask, &g_mask,
		                                &b_mask, &a_mask))
			return SDL_SetError("pixel format setting failed");

		/* get dimensions */
		int w, h;
		SDL_GetWindowSize(window, &w, &h);
		SDL_SetWindowResizable(window, (SDL_bool)true);

		/* allocate and attach memory for framebuffer */
		if (drv.fb_mem.constructed())
			drv.fb_mem.destruct();

		drv.fb_mem.construct(global_env().rm(),
		                     drv.framebuffer->dataspace(w, h));

		bool use_double = true;
		if (use_double)
			drv.fb_double.construct(global_env().ram(),
			                        global_env().rm(),
			                        w * h * bpp / 8);

		void * const fb_mem = drv.fb_double.constructed()
		                    ? drv.fb_double->local_addr<void>()
		                    : drv.fb_mem->local_addr<void>();

		surface = SDL_CreateRGBSurfaceFrom(fb_mem, w, h, bpp,
		                                   w * bpp / 8 /* pitch */,
		                                   r_mask, g_mask, b_mask, a_mask);
		if (!surface)
			return SDL_SetError("setting surface failed");

		/* set name and user data */
		SDL_SetWindowData(window, surface_name, surface);

		*format = surface_format;
		*pixels = surface->pixels;
		*pitch  = surface->pitch;

		/* set focus to window */
		SDL_SetMouseFocus(window);

		return 0;
	}

	static int Genode_Fb_UpdateWindowFramebuffer(SDL_VideoDevice * const device,
	                                             SDL_Window * const window,
	                                             SDL_Rect const * const rects,
	                                             int const num_rects)
	{
		if (!device || !device->driverdata)
			return SDL_SetError("invalid pointer");;

		Genode_Driverdata &drv = *(Genode_Driverdata *)device->driverdata;

		SDL_Surface *surface = (SDL_Surface *) SDL_GetWindowData(window,
		                                                         surface_name);
		if (!surface)
			return SDL_SetError("Could not get surface for window");

		for(int i = 0; i < num_rects; i++) {

			if (drv.fb_double.constructed()) {
				memcpy(drv.fb_mem->local_addr<void>(),
				       drv.fb_double->local_addr<void>(),
				       drv.fb_double->size());
			}

			drv.framebuffer->refresh({ { rects[i].x, rects[i].y },
			                           { rects[i].w, rects[i].h } });
		}

		return 0;
	}

	static void Genode_Fb_DestroyWindowFramebuffer(SDL_VideoDevice * const,
	                                               SDL_Window * const window)
	{
		SDL_Surface *surface = (SDL_Surface *)SDL_SetWindowData(window,
		                                                        surface_name,
		                                                        NULL);
		if (surface)
			SDL_FreeSurface(surface);
	}

	static void GenodeVideo_Quit(SDL_VideoDevice * const)
	{
		/* revert device->displays structures ? */
	}

	static int GenodeVideo_Init(SDL_VideoDevice * const device)
	{
		if (!device || !device->driverdata)
			return SDL_SetError("invalid pointer");;

		Genode_Driverdata &drv = *(Genode_Driverdata *)device->driverdata;

		if (!drv.framebuffer.constructed()) {
			Genode::error("framebuffer not initialized");
			return -1;
		}

		drv.scr_mode = drv.framebuffer->mode();

		/* set mode specific values */
		device->displays = (SDL_VideoDisplay *)(SDL_calloc(1, sizeof(*device->displays)));
		if (!device->displays)
			return SDL_SetError("Memory allocation failed");

		SDL_DisplayMode mode {
			.format = SDL_PIXELFORMAT_ARGB8888,
			.w = drv.scr_mode.w,
			.h = drv.scr_mode.h,
			.refresh_rate = 0,
			.driverdata = nullptr
		};

		SDL_VideoDisplay &display = device->displays[0];
		if (!SDL_AddDisplayMode(&display, &mode))
			return SDL_SetError("Setting display mode failed");

		display.current_mode = mode;
		device->num_displays = 1;

		return 0;
	}

	static int Genode_CreateWindow(_THIS, SDL_Window * window)
	{

		if (!_this || !window)
			return SDL_SetError("invalid pointer");

		Genode_Driverdata &drv = *(Genode_Driverdata *)_this->driverdata;

		Uint32 const surface_format = SDL_PIXELFORMAT_ARGB8888;

		/* Free the old surface */
		SDL_Surface *surface = (SDL_Surface *)SDL_GetWindowData(window,
		                                                        surface_name);
		if (surface) {
			SDL_SetWindowData(window, surface_name, NULL);
			SDL_FreeSurface(surface);
			surface = NULL;
		}

		/* get 32-bit RGB mask values */
		int bpp;
		Uint32 r_mask, g_mask, b_mask, a_mask;
		if (!SDL_PixelFormatEnumToMasks(surface_format, &bpp, &r_mask, &g_mask,
		                                &b_mask, &a_mask))
			return SDL_SetError("pixel format setting failed");

		/* get dimensions */
		int w, h;
		SDL_GetWindowSize(window, &w, &h);

		/* allocate and attach memory for framebuffer */
		if (drv.fb_mem.constructed())
			drv.fb_mem.destruct();

		drv.fb_mem.construct(global_env().rm(),
		                     drv.framebuffer->dataspace(w, h));

		void * const fb_mem = drv.fb_mem->local_addr<void>();

		/* set name and user data */
		SDL_SetWindowData(window, surface_name, surface);
		SDL_SetWindowResizable(window, (SDL_bool)true);

#if defined(SDL_VIDEO_OPENGL_EGL)
		if (window->flags & SDL_WINDOW_OPENGL) {

			drv.egl_window.addr = (unsigned char *)fb_mem;
			drv.egl_window.width = w;
			drv.egl_window.height = h;

			drv.egl_surface = SDL_EGL_CreateSurface(_this, (NativeWindowType) &drv.egl_window);

			if (drv.egl_surface == EGL_NO_SURFACE) {
				return SDL_SetError("failed to create an EGL window surface");
			}
		}
#endif

		/* set focus to window */
		SDL_SetMouseFocus(window);

		return 0;
	}


	static SDL_VideoDevice *GenodeVideo_CreateDevice(int const devindex)
	{
		SDL_VideoDevice   *device;
		Genode_Driverdata *data;

		device = (SDL_VideoDevice*) SDL_calloc(1, sizeof(SDL_VideoDevice));
		if (!device) {
			SDL_OutOfMemory();
			return nullptr;
		}

		data = (Genode_Driverdata*) SDL_calloc(1, sizeof(Genode_Driverdata));
		if (!data) {
			SDL_free(device);
			SDL_OutOfMemory();
			return nullptr;
		}

		data->framebuffer.construct(global_env(), global_gui());

		device->driverdata = data;

		/* video */
		device->VideoInit = GenodeVideo_Init;
		device->VideoQuit = GenodeVideo_Quit;
		device->free      = GenodeVideo_DeleteDevice;

		/* framebuffer */
		device->CreateWindowFramebuffer  = Genode_Fb_CreateWindowFramebuffer;
		device->UpdateWindowFramebuffer  = Genode_Fb_UpdateWindowFramebuffer;
		device->DestroyWindowFramebuffer = Genode_Fb_DestroyWindowFramebuffer;

		device->CreateSDLWindow = Genode_CreateWindow;

#if defined(SDL_VIDEO_OPENGL_EGL)
		device->GL_SwapWindow      = Genode_GLES_SwapWindow;
		device->GL_GetSwapInterval = Genode_GLES_GetSwapInterval;
		device->GL_SetSwapInterval = Genode_GLES_SetSwapInterval;
		device->GL_GetDrawableSize = Genode_GLES_GetDrawableSize;
		device->GL_MakeCurrent     = Genode_GLES_MakeCurrent;
		device->GL_CreateContext   = Genode_GLES_CreateContext;
		device->GL_LoadLibrary     = Genode_GLES_LoadLibrary;
		device->GL_UnloadLibrary   = Genode_GLES_UnloadLibrary;
		device->GL_GetProcAddress  = Genode_GLES_GetProcAddress;
		device->GL_DeleteContext   = Genode_GLES_DeleteContext;
#endif

		device->PumpEvents       = Genode_Fb_PumpEvents;

		return device;
	}

#if defined(SDL_VIDEO_OPENGL_EGL)
	int Genode_GLES_LoadLibrary(_THIS, char const *path)
	{
		int ret;
		Genode_Driverdata &drv = *(Genode_Driverdata *)_this->driverdata;

		ret = SDL_EGL_LoadLibrary(_this, path, EGL_DEFAULT_DISPLAY, 0);
		return ret;
	}

	SDL_GLContext Genode_GLES_CreateContext(_THIS, SDL_Window *window)
	{
		SDL_GLContext context;
		Genode_Driverdata &drv = *(Genode_Driverdata *)_this->driverdata;
		context = SDL_EGL_CreateContext(_this, drv.egl_surface);
		return context;
	}

	int Genode_GLES_SwapWindow(_THIS, SDL_Window *window)
	{
		Genode_Driverdata &drv = *(Genode_Driverdata *)_this->driverdata;

		if (SDL_EGL_SwapBuffers(_this, drv.egl_surface) < 0) {
			return -1;
		}
		drv.framebuffer->refresh({ { 0, 0 }, { drv.egl_window.width, drv.egl_window.height } });
		return 0;
	}

	int Genode_GLES_MakeCurrent(_THIS, SDL_Window * window, SDL_GLContext context)
	{
		int ret;

		if (window && context) {
			Genode_Driverdata &drv = *(Genode_Driverdata *)_this->driverdata;
			ret = SDL_EGL_MakeCurrent(_this, drv.egl_surface, context);
		}
		else {
			ret = SDL_EGL_MakeCurrent(_this, NULL, NULL);
		}

		return ret;
	}

	void Genode_GLES_GetDrawableSize(_THIS, SDL_Window * window, int * w, int * h)
	{
		Genode_Driverdata &drv = *(Genode_Driverdata *)_this->driverdata;
		if (window->driverdata) {
			if (w) {
				*w = drv.egl_window.width;
			}

			if (h) {
				*h = drv.egl_window.height;
			}
		}
	}

	void Genode_GLES_DeleteContext(_THIS, SDL_GLContext context)
	{
		SDL_EGL_DeleteContext(_this, context);
	}

#endif

	VideoBootStrap GenodeVideo_bootstrap = {
		"Genode_Fb", "SDL Genode Framebuffer video driver",
		GenodeVideo_Available, GenodeVideo_CreateDevice
	};

} //extern "C"
