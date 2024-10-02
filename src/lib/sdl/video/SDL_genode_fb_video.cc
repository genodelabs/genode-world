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
#include <base/log.h>
#include <base/env.h>
#include <gui_session/connection.h>

/* local includes */
#include <SDL_genode_internal.h>


extern Genode::Env     &global_env();
extern Gui::Connection &global_gui();

extern Genode::Mutex event_mutex;
extern Video         video_events;


extern "C" {

#include <dlfcn.h>

#include <SDL/SDL.h>
#include <SDL/SDL_video.h>
#include <SDL/SDL_mouse.h>
#include "SDL_sysvideo.h"
#include "SDL_pixels_c.h"
#include "SDL_events_c.h"
#include "SDL_genode_fb_events.h"
#include "SDL_genode_fb_video.h"

	static SDL_Rect df_mode;

	static Gui::Rect scr_rect;
	static SDL_Rect *modes[2];

	struct Sdl_framebuffer
	{
		Genode::Env        &_env;
		Gui::Connection    &_gui;
		Gui::Top_level_view _view { _gui };
		Genode::Semaphore   _scr_valid_sem { };

		Gui::Rect _gui_window()
		{
			return _gui.window().convert<Gui::Rect>(
				[&] (Gui::Rect rect) { return rect; },
				[&] (Gui::Undefined) { return _gui.panorama().convert<Gui::Rect>(
					[&] (Gui::Rect rect) { return rect; },
					[&] (Gui::Undefined) { return Gui::Rect { }; }); });
		}

		void _handle_mode_change()
		{
			Gui::Rect const rect = _gui_window();
			if (!rect.valid())
				return;

			scr_rect = rect;

			df_mode.w = scr_rect.w();
			df_mode.h = scr_rect.h();

			{
				Genode::Mutex::Guard guard(event_mutex);

				video_events.resize_pending = true;
				video_events.width  = scr_rect.w();
				video_events.height = scr_rect.h();
			}

			_scr_valid_sem.up();
		}

		Genode::Io_signal_handler<Sdl_framebuffer> _mode_handler {
			_env.ep(), *this, &Sdl_framebuffer::_handle_mode_change };

		Sdl_framebuffer(Genode::Env &env, Gui::Connection &gui)
		:
			_env(env), _gui(gui)
		{
			_gui.info_sigh(_mode_handler);
			_view.front();

			while (!scr_rect.valid())
				_scr_valid_sem.down();
		}


		/************************************
		 ** Framebuffer::Session Interface **
		 ************************************/

		Genode::Dataspace_capability dataspace(unsigned width, unsigned height)
		{
			_gui.buffer(::Framebuffer::Mode { .area  = { width, height },
			                                  .alpha = false });

			::Framebuffer::Mode mode = _gui.framebuffer.mode();

			_view.area({ Genode::min(mode.area.w, width),
			             Genode::min(mode.area.h, height) });

			return _gui.framebuffer.dataspace();
		}

		void refresh(Framebuffer::Rect rect) {
			_gui.framebuffer.refresh(rect); }

		void title(char const *string)
		{
			_gui.enqueue<Gui::Session::Command::Title>(_view.id(), string);
			_gui.execute();
		}
	};

	static Genode::Constructible<Sdl_framebuffer> framebuffer;

#if defined(SDL_VIDEO_OPENGL)

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglplatform.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#define MAX_CONFIGS 10
#define MAX_MODES 100

	/**********************************
	 ** EGL/OpenGL backend functions **
	 **********************************/

	static EGLDisplay display;
	static EGLSurface screen_surf;
	static EGLNativeWindowType native_window;

	typedef EGLBoolean  (*eglBindAPI_func)             (EGLenum);
	typedef EGLBoolean  (*eglChooseConfig_func)        (EGLDisplay, const EGLint *, EGLConfig *, EGLint , EGLint *);
	typedef EGLContext  (*eglCreateContext_func)       (EGLDisplay, EGLConfig, EGLContext, const EGLint *);
	typedef EGLSurface  (*eglCreatePixmapSurface_func) (EGLDisplay, EGLConfig, EGLNativePixmapType, const EGLint *);
	typedef EGLDisplay  (*eglGetDisplay_func)          (EGLNativeDisplayType);
	typedef EGLBoolean  (*eglInitialize_func)          (EGLDisplay, EGLint *, EGLint *);
	typedef EGLBoolean  (*eglMakeCurrent_func)         (EGLDisplay, EGLSurface, EGLSurface, EGLContext);
	typedef EGLBoolean  (*eglSwapBuffers_func)         (EGLDisplay, EGLSurface);
	typedef EGLBoolean  (*eglWaitClient_func)          (void);
	typedef char const* (*eglQueryString_func)         (EGLDisplay, EGLint);
	typedef __eglMustCastToProperFunctionPointerType
	                    (*eglGetProcAddress_func)      (const char *procname);


	static eglBindAPI_func             __eglBindAPI;
	static eglChooseConfig_func        __eglChooseConfig;
	static eglCreateContext_func       __eglCreateContext;
	static eglCreatePixmapSurface_func __eglCreatePixmapSurface;
	static eglGetDisplay_func          __eglGetDisplay;
	static eglInitialize_func          __eglInitialize;
	static eglMakeCurrent_func         __eglMakeCurrent;
	static eglSwapBuffers_func         __eglSwapBuffers;
	static eglWaitClient_func          __eglWaitClient;
	static eglQueryString_func         __eglQueryString;
	static eglGetProcAddress_func      __eglGetProcAddress;

	static bool init_egl()
	{
		void *egl = dlopen("egl.lib.so", 0);
		if (!egl) {
			Genode::error("could not open EGL library");
			return false;
		}

#define LOAD_GL_FUNC(lib, sym) \
	__ ## sym = (sym ##_func) dlsym(lib, #sym); \
	if (!__ ## sym) { return false; }

		LOAD_GL_FUNC(egl, eglBindAPI)
		LOAD_GL_FUNC(egl, eglChooseConfig)
		LOAD_GL_FUNC(egl, eglCreateContext)
		LOAD_GL_FUNC(egl, eglCreatePixmapSurface)
		LOAD_GL_FUNC(egl, eglGetDisplay)
		LOAD_GL_FUNC(egl, eglInitialize)
		LOAD_GL_FUNC(egl, eglMakeCurrent)
		LOAD_GL_FUNC(egl, eglQueryString)
		LOAD_GL_FUNC(egl, eglSwapBuffers)
		LOAD_GL_FUNC(egl, eglWaitClient)
		LOAD_GL_FUNC(egl, eglGetProcAddress)

#undef LOAD_GL_FUNC

		return true;
	}

	static bool init_opengl(SDL_VideoDevice *t)
	{
		if (!init_egl()) { return false; }

		int maj, min;
		EGLContext ctx;
		EGLConfig configs[MAX_CONFIGS];
		GLboolean printInfo = GL_FALSE;

		display = __eglGetDisplay(EGL_DEFAULT_DISPLAY);
		if (!display) {
			Genode::error("eglGetDisplay failed\n");
			return false;
		}

		if (!__eglInitialize(display, &maj, &min)) {
			Genode::error("eglInitialize failed\n");
			return false;
		}

		Genode::log("EGL version = ", maj, ".", min);
		Genode::log("EGL_VENDOR = ", __eglQueryString(display, EGL_VENDOR));

		EGLConfig config;
		EGLint config_attribs[32];
		EGLint renderable_type, num_configs, i;

		i = 0;
		config_attribs[i++] = EGL_RED_SIZE;
		config_attribs[i++] = 1;
		config_attribs[i++] = EGL_GREEN_SIZE;
		config_attribs[i++] = 1;
		config_attribs[i++] = EGL_BLUE_SIZE;
		config_attribs[i++] = 1;
		config_attribs[i++] = EGL_DEPTH_SIZE;
		config_attribs[i++] = 1;

		config_attribs[i++] = EGL_SURFACE_TYPE;
		config_attribs[i++] = EGL_WINDOW_BIT;;

		config_attribs[i++] = EGL_RENDERABLE_TYPE;
		renderable_type = 0x0;
		renderable_type |= EGL_OPENGL_BIT;
		config_attribs[i++] = renderable_type;

		config_attribs[i] = EGL_NONE;

		if (!__eglChooseConfig(display, config_attribs, &config, 1, &num_configs)
		    || !num_configs) {
			Genode::error("eglChooseConfig failed");
			return false;
		}

		__eglBindAPI(EGL_OPENGL_API);

		EGLint context_attribs[4]; context_attribs[0] = EGL_NONE;
		ctx = __eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribs);
		if (!ctx) {
			Genode::error("eglCreateContext failed");
			return false;
		}

		Genode_egl_window egl_window { (int)scr_rect.w(), (int)scr_rect.h(),
		                               (unsigned char*)t->hidden->buffer };

		screen_surf = __eglCreatePixmapSurface(display, config, &egl_window, NULL);
		if (screen_surf == EGL_NO_SURFACE) {
			Genode::error("eglCreatePixmapSurface failed");
			return false;
		}

		if (!__eglMakeCurrent(display, screen_surf, screen_surf, ctx)) {
			Genode::error("eglMakeCurrent failed");
			return false;
		}

		t->gl_config.driver_loaded = 1;
		return true;
	}
#endif

	/****************************************
	 * Genode_Fb driver bootstrap functions *
	 ****************************************/

	static int Genode_Fb_Available(void)
	{
		if (!framebuffer.constructed()) {
			framebuffer.construct(global_env(), global_gui());
		}

		return 1;
	}

	static void Genode_Fb_DeleteDevice(SDL_VideoDevice *device)
	{
		if (framebuffer.constructed())
			framebuffer.destruct();
	}

	static void Genode_Fb_SetCaption(SDL_VideoDevice *device, const char *title, const char *icon)
	{
		if (title && framebuffer.constructed())
			framebuffer->title(title);
	}

	static SDL_VideoDevice *Genode_Fb_CreateDevice(int devindex)
	{
		SDL_VideoDevice *device;

		/* Initialize all variables that we clean on shutdown */
		device = (SDL_VideoDevice *)SDL_malloc(sizeof(SDL_VideoDevice));
		if ( device ) {
			SDL_memset(device, 0, (sizeof *device));
			device->hidden = (struct SDL_PrivateVideoData *)
				SDL_malloc((sizeof *device->hidden));
		}
		if ( (device == 0) || (device->hidden == 0) ) {
			SDL_OutOfMemory();
			if ( device ) {
				SDL_free(device);
			}
			return(0);
		}
		SDL_memset(device->hidden, 0, (sizeof *device->hidden));

		/* Set the function pointers */
		device->VideoInit        = Genode_Fb_VideoInit;
		device->ListModes        = Genode_Fb_ListModes;
		device->SetVideoMode     = Genode_Fb_SetVideoMode;
		device->SetColors        = Genode_Fb_SetColors;
		device->UpdateRects      = Genode_Fb_UpdateRects;
		device->VideoQuit        = Genode_Fb_VideoQuit;
		device->AllocHWSurface   = Genode_Fb_AllocHWSurface;
		device->LockHWSurface    = Genode_Fb_LockHWSurface;
		device->UnlockHWSurface  = Genode_Fb_UnlockHWSurface;
		device->FreeHWSurface    = Genode_Fb_FreeHWSurface;
		device->InitOSKeymap     = Genode_Fb_InitOSKeymap;
		device->PumpEvents       = Genode_Fb_PumpEvents;
		device->free             = Genode_Fb_DeleteDevice;
		device->CreateYUVOverlay = 0;
		device->CheckHWBlit      = 0;
		device->FillHWRect       = 0;
		device->SetHWColorKey    = 0;
		device->SetHWAlpha       = 0;
		device->FlipHWSurface    = 0;
		device->SetCaption       = Genode_Fb_SetCaption;
		device->SetIcon          = 0;
		device->IconifyWindow    = 0;
		device->GrabInput        = 0;
		device->GetWMInfo        = 0;

		device->GL_MakeCurrent    = Genode_Fb_GL_MakeCurrent;
		device->GL_SwapBuffers    = Genode_Fb_GL_SwapBuffers;
		device->GL_LoadLibrary    = Genode_Fb_GL_LoadLibrary;
		device->GL_GetProcAddress = Genode_Fb_GL_GetProcAddress;
		return device;
	}


	VideoBootStrap Genode_fb_bootstrap = {
		"Genode_Fb", "SDL genode_fb video driver",
		Genode_Fb_Available, Genode_Fb_CreateDevice
	};


	/*****************
	 * Functionality
	 ****************/

	/**
	 * Initialize the native video subsystem, filling 'vformat' with the
	 * "best" display pixel format, returning 0 or -1 if there's an error.
	 */
	int Genode_Fb_VideoInit(SDL_VideoDevice *t, SDL_PixelFormat *vformat)
	{
		if (!framebuffer.constructed()) {
			Genode::error("framebuffer not initialized");
			return -1;
		}

		t->info.current_w = scr_rect.w();
		t->info.current_h = scr_rect.h();
		Genode::log("Framebuffer has "
		            "width=",  t->info.current_w, " "
		            "height=", t->info.current_h);

		/* set mode specific values */
		vformat->BitsPerPixel  = 32;
		vformat->BytesPerPixel = sizeof(Genode::Pixel_rgb888);
		vformat->Rmask = 0x00ff0000;
		vformat->Gmask = 0x0000ff00;
		vformat->Bmask = 0x000000ff;
		modes[0] = &df_mode;
		df_mode.w = scr_rect.w();
		df_mode.h = scr_rect.h();
		modes[1] = 0;

		t->hidden->buffer = 0;
		return 0;
	}


	/**
	 *Note:  If we are terminated, this could be called in the middle of
	 * another SDL video routine -- notably UpdateRects.
	 */
	void Genode_Fb_VideoQuit(SDL_VideoDevice *t)
	{
		Genode::log("Quit video device ...");

		if (t->screen->pixels) {
			t->screen->pixels = nullptr;
		}

		if (t->hidden->buffer) {
			global_env().rm().detach(Genode::addr_t(t->hidden->buffer));
			t->hidden->buffer = nullptr;
		}
	}


	/**
	 * Any mode is okay if the pixel format is 32 bit
	 */
	SDL_Rect **Genode_Fb_ListModes(SDL_VideoDevice *t,
	                               SDL_PixelFormat *format,
	                               Uint32 flags)
	{
		return (SDL_Rect **)((format->BitsPerPixel == 32) ? -1L : 0L);
	}


	/**
	 * Set the requested video mode, returning a surface which will be
	 * set to the SDL_VideoSurface.  The width and height will already
	 * be verified by ListModes(), and the video subsystem is free to
	 * set the mode to a supported bit depth different from the one
	 * specified -- the desired bpp will be emulated with a shadow
	 * surface if necessary.  If a new mode is returned, this function
	 * should take care of cleaning up the current mode.
	 */
	SDL_Surface *Genode_Fb_SetVideoMode(SDL_VideoDevice *t,
	                                    SDL_Surface *current,
	                                    int width, int height,
	                                    int bpp, Uint32 flags)
	{
		/* for now we do not support this */
		if (t->hidden->buffer && flags & SDL_OPENGL) {
			Genode::error("resizing a OpenGL window not possible");
			return nullptr;
		}

		/*
		 * XXX there is something wrong with how this is used now.
		 *     SDL_Flip() is going to call FlipHWSurface which was never
		 *     implemented and leads to a nullptr function call.
		 */
		if (flags & SDL_DOUBLEBUF) {
			Genode::warning("disable requested double-buffering");
			flags &= ~SDL_DOUBLEBUF;
		}

		/* Map the buffer */
		Genode::Dataspace_capability fb_ds_cap =
			framebuffer->dataspace(width, height);
		if (!fb_ds_cap.valid()) {
			Genode::error("could not request dataspace for frame buffer");
			return nullptr;
		}

		if (t->hidden->buffer) {
			global_env().rm().detach(Genode::addr_t(t->hidden->buffer));
		}

		t->hidden->buffer = global_env().rm().attach(fb_ds_cap, {
			.size       = { },  .offset    = { },
			.use_at     = { },  .at        = { },
			.executable = { },  .writeable = true
		}).convert<void *>(
			[&] (Genode::Region_map::Range range)  { return (void *)range.start; },
			[&] (Genode::Region_map::Attach_error) { return nullptr; }
		);

		if (!t->hidden->buffer) {
			Genode::error("no buffer for requested mode");
			return nullptr;
		}

		Genode::log("Set video mode to: ", width, "x", height, "@", bpp);

		SDL_memset(t->hidden->buffer, 0, width * height * (bpp / 8));

		if (!SDL_ReallocFormat(current, bpp, 0, 0, 0, 0) ) {
			Genode::error("couldn't allocate new pixel format for requested mode");
			return nullptr;
		}

		/* Set up the new mode framebuffer */
		current->flags = flags | SDL_FULLSCREEN;
		t->hidden->w = current->w = width;
		t->hidden->h = current->h = height;
		current->pitch = current->w * (bpp / 8);

#if defined(SDL_VIDEO_OPENGL)
		if ((flags & SDL_OPENGL) && !init_opengl(t)) {
			return nullptr;
		}
#endif

		/*
		 * XXX if SDL ever wants to free the pixels pointer,
		 *     free() in the libc will trigger a page-fault
		 */
		current->pixels = t->hidden->buffer;
		return current;
	}


	/**
	 * We don't actually allow hardware surfaces other than the main one
	 */
	static int Genode_Fb_AllocHWSurface(SDL_VideoDevice *t,
	                                    SDL_Surface *surface)
	{
		Genode::log(__func__, " not supported yet ...");
		return -1;
	}


	static void Genode_Fb_FreeHWSurface(SDL_VideoDevice *t,
	                                    SDL_Surface *surface)
	{
		Genode::log(__func__, " not supported yet ...");
	}


	/**
	 * We need to wait for vertical retrace on page flipped displays
	 */
	static int Genode_Fb_LockHWSurface(SDL_VideoDevice *t,
	                                   SDL_Surface *surface)
	{
		/* Genode::log(__func__, " not supported yet ..."); */
		return 0;
	}


	static void Genode_Fb_UnlockHWSurface(SDL_VideoDevice *t,
	                                      SDL_Surface *surface)
	{
		/* Genode::log(__func__, " not supported yet ..."); */
	}


	static void Genode_Fb_UpdateRects(SDL_VideoDevice *t, int numrects,
	                                  SDL_Rect *rects)
	{
		for(unsigned i = 0; i < numrects; i++)
			framebuffer->refresh({ { rects[i].x, rects[i].y },
			                       { rects[i].w, rects[i].h } });
	}


	/**
	 * Sets the color entries { firstcolor .. (firstcolor+ncolors-1) }
	 * of the physical palette to those in 'colors'. If the device is
	 * using a software palette (SDL_HWPALETTE not set), then the
	 * changes are reflected in the logical palette of the screen
	 * as well.
	 * The return value is 1 if all entries could be set properly
	 * or 0 otherwise.
	 */
	int Genode_Fb_SetColors(SDL_VideoDevice *t, int firstcolor,
	                        int ncolors, SDL_Color *colors)
	{
		Genode::warning(__func__, " not yet implemented");
		return 1;
	}


	int Genode_Fb_GL_MakeCurrent(SDL_VideoDevice *t)
	{
		Genode::warning(__func__, ": not yet implemented");
		return 0;
	}


	void Genode_Fb_GL_SwapBuffers(SDL_VideoDevice *t)
	{
#if defined(SDL_VIDEO_OPENGL)
		__eglWaitClient();
		__eglSwapBuffers(display, screen_surf);
		framebuffer->refresh(scr_rect);
#endif
	}

	int Genode_Fb_GL_LoadLibrary(SDL_VideoDevice *t, const char *path)
	{
		Genode::warning(__func__, ": not yet implemented");
		return 0;
	}


	void* Genode_Fb_GL_GetProcAddress(SDL_VideoDevice *t, const char *proc) {

		return (void*)__eglGetProcAddress(proc); }
} //extern "C"
