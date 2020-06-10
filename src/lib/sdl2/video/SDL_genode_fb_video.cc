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
#include <base/env.h>
#include <base/log.h>
#include <nitpicker_session/connection.h>

/* local includes */
#include <SDL_genode_internal.h>


extern Genode::Env           &global_env();
extern Nitpicker::Connection &global_nitpicker();

extern Genode::Lock event_lock;
extern Video        video_events;


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

	struct Sdl_framebuffer
	{
		Genode::Env           &_env;
		Nitpicker::Connection &_nitpicker;
		Nitpicker::Session::View_handle _view {
			_nitpicker.create_view() };

		void _handle_mode_change()
		{
			Genode::Lock_guard<Genode::Lock> guard(event_lock);

			Framebuffer::Mode mode = _nitpicker.mode();

			video_events.resize_pending = true;
			video_events.width  = mode.width();
			video_events.height = mode.height();
		}

		Genode::Signal_handler<Sdl_framebuffer> _mode_handler {
			_env.ep(), *this, &Sdl_framebuffer::_handle_mode_change };

		Sdl_framebuffer(Genode::Env &env, Nitpicker::Connection &nitpicker)
		:
			_env(env), _nitpicker(nitpicker)
		{
			_nitpicker.mode_sigh(_mode_handler);

			using namespace Nitpicker;
			_nitpicker.enqueue<Session::Command::To_front>(_view, Session::View_handle());
			_nitpicker.execute();
		}

		~Sdl_framebuffer()
		{
			/* clean up and reduce noise about invalid signals */
			_nitpicker.mode_sigh(Genode::Signal_context_capability());
			dataspace(0, 0);
			_nitpicker.destroy_view(_view);
		}

		/************************************
		 ** Framebuffer::Session Interface **
		 ************************************/

		Genode::Dataspace_capability dataspace(int width, int height)
		{
			_nitpicker.buffer(
				::Framebuffer::Mode(width, height, Framebuffer::Mode::RGB565),
				false);

			::Framebuffer::Mode mode = _nitpicker.framebuffer()->mode();

			using namespace Nitpicker;
			Area area(
				Genode::min(mode.width(), width),
				Genode::min(mode.height(), height));

			typedef Nitpicker::Session::Command Command;
			_nitpicker.enqueue<Command::Geometry>(
				_view, Rect(Point(0, 0), area));
			_nitpicker.execute();

			return _nitpicker.framebuffer()->dataspace();
		}

		Framebuffer::Mode mode() const {
			return _nitpicker.mode(); }

		void refresh(int x, int y, int w, int h) {
			_nitpicker.framebuffer()->refresh(x, y, w, h); }

		void title(char const *string)
		{
			_nitpicker.enqueue<Nitpicker::Session::Command::Title>(_view, string);
			_nitpicker.execute();
		}
	};

	static char const * const surface_name = "genode_surface";

	struct Genode_Driverdata
	{
		Genode::Constructible<Sdl_framebuffer>                framebuffer;
		Genode::Constructible<Genode::Attached_dataspace>     fb_mem;
		Genode::Constructible<Genode::Attached_ram_dataspace> fb_double;
		Framebuffer::Mode                                     scr_mode;
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

		Uint32 const surface_format = SDL_PIXELFORMAT_RGB565;

		/* Free the old surface */
		SDL_Surface *surface = (SDL_Surface *)SDL_GetWindowData(window,
		                                                        surface_name);
		if (surface) {
			SDL_SetWindowData(window, surface_name, NULL);
			SDL_FreeSurface(surface);
			surface = NULL;
		}

		/* get 16bit RGB mask values */
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

			drv.framebuffer->refresh(rects[i].x, rects[i].y,
			                         rects[i].w, rects[i].h);
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

		/* Get the framebuffer size and mode infos */
		drv.scr_mode = drv.framebuffer->mode();

		/* set mode specific values */
		switch(drv.scr_mode.format()) {
		case Framebuffer::Mode::RGB565:
		{
			Genode::log("We use pixelformat rgb565.");

			device->displays = (SDL_VideoDisplay *)(SDL_calloc(1, sizeof(*device->displays)));
			if (!device->displays)
				return SDL_SetError("Memory allocation failed");

			SDL_DisplayMode mode {
				.format = SDL_PIXELFORMAT_RGB565,
				.w = drv.scr_mode.width(),
				.h = drv.scr_mode.height(),
				.refresh_rate = 0,
				.driverdata = nullptr
			};

			SDL_VideoDisplay &display = device->displays[0];
			if (!SDL_AddDisplayMode(&display, &mode))
				return SDL_SetError("Setting display mode failed");

			display.current_mode = mode;
			device->num_displays = 1;

			break;
		}
		default:
			SDL_SetError("Couldn't get console mode info");
			GenodeVideo_Quit(device);
			return -1;
		}

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

		data->framebuffer.construct(global_env(), global_nitpicker());

		device->driverdata = data;

		/* video */
		device->VideoInit = GenodeVideo_Init;
		device->VideoQuit = GenodeVideo_Quit;
		device->free      = GenodeVideo_DeleteDevice;

		/* framebuffer */
		device->CreateWindowFramebuffer  = Genode_Fb_CreateWindowFramebuffer;
		device->UpdateWindowFramebuffer  = Genode_Fb_UpdateWindowFramebuffer;
		device->DestroyWindowFramebuffer = Genode_Fb_DestroyWindowFramebuffer;

		device->PumpEvents       = Genode_Fb_PumpEvents;

		return device;
	}

	VideoBootStrap GenodeVideo_bootstrap = {
		"Genode_Fb", "SDL Genode Framebuffer video driver",
		GenodeVideo_Available, GenodeVideo_CreateDevice
	};

} //extern "C"
