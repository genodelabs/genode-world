/*
 * \brief  Genode-specific event backend
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
#include <gui_session/connection.h>
#include <base/log.h>
#include <input_session/connection.h>
#include <input/event.h>
#include <input/keycodes.h>

/* local includes */
#include <SDL_genode_internal.h>


Genode::Mutex event_mutex;
Video video_events;


static Genode::Env *_global_env = nullptr;

static Genode::Constructible<Gui::Connection> _global_gui { };


Genode::Env &global_env()
{
	if (!_global_env) {
		Genode::error("sdl_init_genode() not called, aborting");
		throw Genode::Exception();
	}

	return *_global_env;
}


Gui::Connection &global_gui()
{
	if (!_global_gui.constructed())
		_global_gui.construct(global_env());

	return *_global_gui;
}


void sdl_init_genode(Genode::Env &env)
{
	_global_env = &env;
}


extern "C" {

#include <SDL2/SDL.h>
#include "SDL_events_c.h"
#include "SDL_sysevents.h"
#include "SDL_genode_fb_events.h"

	static bool skipkey(Input::Keycode const keycode)
	{
		using namespace Input;

		/* just filter some keys that might wreck havoc */
		switch (keycode) {
		case KEY_CAPSLOCK:   return true;
		case KEY_LEFTALT:    return true;
		case KEY_LEFTCTRL:   return true;
		case KEY_LEFTSHIFT:  return true;
		case KEY_RIGHTALT:   return true;
		case KEY_RIGHTCTRL:  return true;
		case KEY_RIGHTSHIFT: return true;
		default: return false;
		}
	}

	/* "borrowed" from SDL_windowsevents.c */
	static int ConvertUTF32toUTF8(unsigned int codepoint, char *text)
	{
		if (codepoint <= 0x7F) {
			text[0] = (char) codepoint;
			text[1] = '\0';
		} else if (codepoint <= 0x7FF) {
			text[0] = 0xC0 | (char) ((codepoint >> 6) & 0x1F);
			text[1] = 0x80 | (char) (codepoint & 0x3F);
			text[2] = '\0';
		} else if (codepoint <= 0xFFFF) {
			text[0] = 0xE0 | (char) ((codepoint >> 12) & 0x0F);
			text[1] = 0x80 | (char) ((codepoint >> 6) & 0x3F);
			text[2] = 0x80 | (char) (codepoint & 0x3F);
			text[3] = '\0';
		} else if (codepoint <= 0x10FFFF) {
			text[0] = 0xF0 | (char) ((codepoint >> 18) & 0x0F);
			text[1] = 0x80 | (char) ((codepoint >> 12) & 0x3F);
			text[2] = 0x80 | (char) ((codepoint >> 6) & 0x3F);
			text[3] = 0x80 | (char) (codepoint & 0x3F);
			text[4] = '\0';
		} else {
			return SDL_FALSE;
		}
		return SDL_TRUE;
	}

	static Genode::Constructible<Input::Session_client> input;
	static SDL_Scancode scancodes[SDL_NUM_SCANCODES];
//	static SDL_Keycode  keymap[SDL_NUM_SCANCODES];
	static int buttonmap[SDL_NUM_SCANCODES];

	static SDL_Scancode getscancode(Input::Keycode const keycode)
	{
		if (keycode < 0 || keycode > sizeof(scancodes) / sizeof(scancodes[0]))
			return SDL_SCANCODE_UNKNOWN;
		return scancodes[keycode];
	}

	void Genode_Fb_PumpEvents(SDL_VideoDevice * const device)
	{
		if (!input.constructed()) /* XXX */ {
			Genode_Fb_InitOSKeymap(device);
			/* there is a default map using the scancode array */
//			SDL_SetKeymap(0, keymap, SDL_NUM_SCANCODES);
		}

		Genode::Mutex::Guard guard(event_mutex);

		SDL_Window * const window   = SDL_GetMouseFocus();

		if (video_events.resize_pending) {
			video_events.resize_pending = false;

			int const width  = video_events.width;
			int const height = video_events.height;

			bool const quit = width == 0 && height == 0;

			if (!quit) {
				/* might force clients to call SDL_GetWindowSurface that,
				 * according to the documentation, may not be done when using
				 * 3D or render APIs. So see how that goes...
				 */
				SDL_SendWindowEvent(window, SDL_WINDOWEVENT_RESIZED, width, height);
			}
			else {
				/* at least try to quit w/o other event handling */
				if (SDL_SendQuit())
					return;
				else
					Genode::warning("could not deliver requested SDL_QUIT event");
			}
		}

		if (!input->pending())
			return;

		SDL_MouseID  const mouse_id = 0;

		input->for_each_event([&] (Input::Event const &curr) {

			curr.handle_absolute_motion([&] (int x, int y) {
				SDL_SendMouseMotion(window, mouse_id, 0 /* !relative */, x, y);
			});

			curr.handle_relative_motion([&] (int x, int y) {
				SDL_SendMouseMotion(window, mouse_id, 1 /* relative */, x, y);
			});

			/* return true if keycode refers to a button */
			auto const mouse_button = [] (Input::Keycode key) {
				return key >= Input::BTN_MISC && key <= Input::BTN_GEAR_UP; };

			curr.handle_press([&] (Input::Keycode key, Genode::Codepoint codepoint) {
				if (mouse_button(key))
					SDL_SendMouseButton(window, mouse_id, SDL_PRESSED, buttonmap[key]);
				else
					SDL_SendKeyboardKey(SDL_PRESSED, getscancode(key));

				if (SDL_EventState(SDL_TEXTINPUT, SDL_QUERY) && !skipkey(key)) {
					char text[SDL_TEXTINPUTEVENT_TEXT_SIZE];
					SDL_zeroa(text);
					if (ConvertUTF32toUTF8(codepoint.value, text)) {
						SDL_SendKeyboardText(text);
					}
				}
			});

			curr.handle_wheel([&] (int const x, int const y) {
				SDL_SendMouseWheel(window, mouse_id, x, y, SDL_MOUSEWHEEL_NORMAL);
			});

			curr.handle_release([&] (Input::Keycode key) {
				if (mouse_button(key))
					SDL_SendMouseButton(window, mouse_id, SDL_RELEASED, buttonmap[key]);
				else
					SDL_SendKeyboardKey(SDL_RELEASED, getscancode(key));
			});
		});
	}


	void Genode_Fb_InitOSKeymap(SDL_VideoDevice *t)
	{
		try {
			input.construct(_global_env->rm(),
			                _global_gui->input_session());
		} catch (...) {
			Genode::error("no input driver available!");
			return;
		}

		using namespace Input;

		/* Prepare button mappings */
		for (int i=0; i<SDL_NUM_SCANCODES; i++)
		{
			switch(i)
			{
			case BTN_LEFT: buttonmap[i]=SDL_BUTTON_LEFT; break;
			case BTN_RIGHT: buttonmap[i]=SDL_BUTTON_RIGHT; break;
			case BTN_MIDDLE: buttonmap[i]=SDL_BUTTON_MIDDLE; break;
			case BTN_0:
			case BTN_1:
			case BTN_2:
			case BTN_3:
			case BTN_4:
			case BTN_5:
			case BTN_6:
			case BTN_7:
			case BTN_8:
			case BTN_9:
			case BTN_SIDE:
			case BTN_EXTRA:
			case BTN_FORWARD:
			case BTN_BACK:
			case BTN_TASK:
			case BTN_TRIGGER:
			case BTN_THUMB:
			case BTN_THUMB2:
			case BTN_TOP:
			case BTN_TOP2:
			case BTN_PINKIE:
			case BTN_BASE:
			case BTN_BASE2:
			case BTN_BASE3:
			case BTN_BASE4:
			case BTN_BASE5:
			case BTN_BASE6:
			case BTN_DEAD:
			case BTN_A:
			case BTN_B:
			case BTN_C:
			case BTN_X:
			case BTN_Y:
			case BTN_Z:
			case BTN_TL:
			case BTN_TR:
			case BTN_TL2:
			case BTN_TR2:
			case BTN_SELECT:
			case BTN_START:
			case BTN_MODE:
			case BTN_THUMBL:
			case BTN_THUMBR:
			case BTN_TOOL_PEN:
			case BTN_TOOL_RUBBER:
			case BTN_TOOL_BRUSH:
			case BTN_TOOL_PENCIL:
			case BTN_TOOL_AIRBRUSH:
			case BTN_TOOL_FINGER:
			case BTN_TOOL_MOUSE:
			case BTN_TOOL_LENS:
			case BTN_TOUCH:
			case BTN_STYLUS:
			case BTN_STYLUS2:
			case BTN_TOOL_DOUBLETAP:
			case BTN_TOOL_TRIPLETAP:
			case BTN_GEAR_DOWN:
			case BTN_GEAR_UP:
			default: buttonmap[i]=0;
			}
		}

		/* Prepare key mappings */
		for(int i = 0; i < SDL_NUM_SCANCODES; i++)
		{
			/* Genode to SDL scancode mappings */
			switch (i)
			{
			case KEY_RESERVED: scancodes[i]=SDL_SCANCODE_UNKNOWN; break;
			case KEY_ESC: scancodes[i]=SDL_SCANCODE_ESCAPE; break;
			case KEY_1: scancodes[i]=SDL_SCANCODE_1; break;
			case KEY_2: scancodes[i]=SDL_SCANCODE_2; break;
			case KEY_3: scancodes[i]=SDL_SCANCODE_3; break;
			case KEY_4: scancodes[i]=SDL_SCANCODE_4; break;
			case KEY_5: scancodes[i]=SDL_SCANCODE_5; break;
			case KEY_6: scancodes[i]=SDL_SCANCODE_6; break;
			case KEY_7: scancodes[i]=SDL_SCANCODE_7; break;
			case KEY_8: scancodes[i]=SDL_SCANCODE_8; break;
			case KEY_9: scancodes[i]=SDL_SCANCODE_9; break;
			case KEY_0: scancodes[i]=SDL_SCANCODE_0; break;
			case KEY_MINUS: scancodes[i]=SDL_SCANCODE_MINUS; break;
			case KEY_EQUAL: scancodes[i]=SDL_SCANCODE_EQUALS; break;
			case KEY_BACKSPACE: scancodes[i]=SDL_SCANCODE_BACKSPACE; break;
			case KEY_TAB: scancodes[i]=SDL_SCANCODE_TAB; break;
			case KEY_Q: scancodes[i]=SDL_SCANCODE_Q; break;
			case KEY_W: scancodes[i]=SDL_SCANCODE_W; break;
			case KEY_E: scancodes[i]=SDL_SCANCODE_E; break;
			case KEY_R: scancodes[i]=SDL_SCANCODE_R; break;
			case KEY_T: scancodes[i]=SDL_SCANCODE_T; break;
			case KEY_Y: scancodes[i]=SDL_SCANCODE_Y; break;
			case KEY_U: scancodes[i]=SDL_SCANCODE_U; break;
			case KEY_I: scancodes[i]=SDL_SCANCODE_I; break;
			case KEY_O: scancodes[i]=SDL_SCANCODE_O; break;
			case KEY_P: scancodes[i]=SDL_SCANCODE_P; break;
			case KEY_LEFTBRACE: scancodes[i]=SDL_SCANCODE_UNKNOWN; break;
			case KEY_RIGHTBRACE: scancodes[i]=SDL_SCANCODE_UNKNOWN; break;
			case KEY_ENTER: scancodes[i]=SDL_SCANCODE_RETURN; break;
			case KEY_LEFTCTRL: scancodes[i]=SDL_SCANCODE_LCTRL; break;
			case KEY_A: scancodes[i]=SDL_SCANCODE_A; break;
			case KEY_S: scancodes[i]=SDL_SCANCODE_S; break;
			case KEY_D: scancodes[i]=SDL_SCANCODE_D; break;
			case KEY_F: scancodes[i]=SDL_SCANCODE_F; break;
			case KEY_G: scancodes[i]=SDL_SCANCODE_G; break;
			case KEY_H: scancodes[i]=SDL_SCANCODE_H; break;
			case KEY_J: scancodes[i]=SDL_SCANCODE_J; break;
			case KEY_K: scancodes[i]=SDL_SCANCODE_K; break;
			case KEY_L: scancodes[i]=SDL_SCANCODE_L; break;
			case KEY_SEMICOLON: scancodes[i]=SDL_SCANCODE_SEMICOLON; break;
			case KEY_APOSTROPHE: scancodes[i]=SDL_SCANCODE_UNKNOWN; break;
			case KEY_GRAVE: scancodes[i]=SDL_SCANCODE_UNKNOWN; break;
			case KEY_LEFTSHIFT: scancodes[i]=SDL_SCANCODE_LSHIFT; break;
			case KEY_BACKSLASH: scancodes[i]=SDL_SCANCODE_BACKSLASH; break;
			case KEY_Z: scancodes[i]=SDL_SCANCODE_Z; break;
			case KEY_X: scancodes[i]=SDL_SCANCODE_X; break;
			case KEY_C: scancodes[i]=SDL_SCANCODE_C; break;
			case KEY_V: scancodes[i]=SDL_SCANCODE_V; break;
			case KEY_B: scancodes[i]=SDL_SCANCODE_B; break;
			case KEY_N: scancodes[i]=SDL_SCANCODE_N; break;
			case KEY_M: scancodes[i]=SDL_SCANCODE_M; break;
			case KEY_COMMA: scancodes[i]=SDL_SCANCODE_COMMA; break;
			case KEY_DOT: scancodes[i]=SDL_SCANCODE_PERIOD; break;
			case KEY_SLASH: scancodes[i]=SDL_SCANCODE_SLASH; break;
			case KEY_RIGHTSHIFT: scancodes[i]=SDL_SCANCODE_RSHIFT; break;
			case KEY_LEFTALT: scancodes[i]=SDL_SCANCODE_LALT; break;
			case KEY_SPACE: scancodes[i]=SDL_SCANCODE_SPACE; break;
			case KEY_CAPSLOCK: scancodes[i]=SDL_SCANCODE_CAPSLOCK; break;
			case KEY_F1: scancodes[i]=SDL_SCANCODE_F1; break;
			case KEY_F2: scancodes[i]=SDL_SCANCODE_F2; break;
			case KEY_F3: scancodes[i]=SDL_SCANCODE_F3; break;
			case KEY_F4: scancodes[i]=SDL_SCANCODE_F4; break;
			case KEY_F5: scancodes[i]=SDL_SCANCODE_F5; break;
			case KEY_F6: scancodes[i]=SDL_SCANCODE_F6; break;
			case KEY_F7: scancodes[i]=SDL_SCANCODE_F7; break;
			case KEY_F8: scancodes[i]=SDL_SCANCODE_F8; break;
			case KEY_F9: scancodes[i]=SDL_SCANCODE_F9; break;
			case KEY_F10: scancodes[i]=SDL_SCANCODE_F10; break;
			case KEY_NUMLOCK: scancodes[i]=SDL_SCANCODE_NUMLOCKCLEAR; break;
			case KEY_SCROLLLOCK: scancodes[i]=SDL_SCANCODE_SCROLLLOCK; break;
			case KEY_KP7: scancodes[i]=SDL_SCANCODE_KP_7; break;
			case KEY_KP8: scancodes[i]=SDL_SCANCODE_KP_8; break;
			case KEY_KP9: scancodes[i]=SDL_SCANCODE_KP_9; break;
			case KEY_KPMINUS: scancodes[i]=SDL_SCANCODE_KP_MINUS; break;
			case KEY_KP4: scancodes[i]=SDL_SCANCODE_KP_4; break;
			case KEY_KP5: scancodes[i]=SDL_SCANCODE_KP_5; break;
			case KEY_KP6: scancodes[i]=SDL_SCANCODE_KP_6; break;
			case KEY_KPPLUS: scancodes[i]=SDL_SCANCODE_KP_PLUS; break;
			case KEY_KP1: scancodes[i]=SDL_SCANCODE_KP_1; break;
			case KEY_KP2: scancodes[i]=SDL_SCANCODE_KP_2; break;
			case KEY_KP3: scancodes[i]=SDL_SCANCODE_KP_3; break;
			case KEY_KP0: scancodes[i]=SDL_SCANCODE_KP_0; break;
			case KEY_KPDOT: scancodes[i]=SDL_SCANCODE_KP_PERIOD; break;
			case KEY_ZENKAKUHANKAKU: scancodes[i]=SDL_SCANCODE_UNKNOWN; break;
			case KEY_102ND: scancodes[i]=SDL_SCANCODE_UNKNOWN; break;
			case KEY_F11: scancodes[i]=SDL_SCANCODE_F11; break;
			case KEY_F12: scancodes[i]=SDL_SCANCODE_F12; break;
			case KEY_RO: scancodes[i]=SDL_SCANCODE_UNKNOWN; break;
			case KEY_KATAKANA: scancodes[i]=SDL_SCANCODE_UNKNOWN; break;
			case KEY_HIRAGANA: scancodes[i]=SDL_SCANCODE_UNKNOWN; break;
			case KEY_HENKAN: scancodes[i]=SDL_SCANCODE_UNKNOWN; break;
			case KEY_KATAKANAHIRAGANA: scancodes[i]=SDL_SCANCODE_UNKNOWN; break;
			case KEY_MUHENKAN: scancodes[i]=SDL_SCANCODE_UNKNOWN; break;
			case KEY_KPJPCOMMA: scancodes[i]=SDL_SCANCODE_UNKNOWN; break;
			case KEY_KPENTER: scancodes[i]=SDL_SCANCODE_KP_ENTER; break;
			case KEY_RIGHTCTRL: scancodes[i]=SDL_SCANCODE_RCTRL; break;
			case KEY_KPSLASH: scancodes[i]=SDL_SCANCODE_KP_DIVIDE; break;
			case KEY_SYSRQ: scancodes[i]=SDL_SCANCODE_SYSREQ; break;
			case KEY_RIGHTALT: scancodes[i]=SDL_SCANCODE_RALT; break;
			case KEY_LINEFEED: scancodes[i]=SDL_SCANCODE_RETURN; break;
			case KEY_HOME: scancodes[i]=SDL_SCANCODE_HOME; break;
			case KEY_UP: scancodes[i]=SDL_SCANCODE_UP; break;
			case KEY_PAGEUP: scancodes[i]=SDL_SCANCODE_PAGEUP; break;
			case KEY_LEFT: scancodes[i]=SDL_SCANCODE_LEFT; break;
			case KEY_RIGHT: scancodes[i]=SDL_SCANCODE_RIGHT; break;
			case KEY_END: scancodes[i]=SDL_SCANCODE_END; break;
			case KEY_DOWN: scancodes[i]=SDL_SCANCODE_DOWN; break;
			case KEY_PAGEDOWN: scancodes[i]=SDL_SCANCODE_PAGEDOWN; break;
			case KEY_INSERT: scancodes[i]=SDL_SCANCODE_INSERT; break;
			case KEY_DELETE: scancodes[i]=SDL_SCANCODE_DELETE; break;
			case KEY_POWER: scancodes[i]=SDL_SCANCODE_POWER; break;
			case KEY_KPEQUAL: scancodes[i]=SDL_SCANCODE_KP_EQUALS; break;

			default: scancodes[i] = SDL_SCANCODE_UNKNOWN; break;
			}
		}
	}
} /* exern "C" */
