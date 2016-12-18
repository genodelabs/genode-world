/*
 * \brief  Libretro input mapping
 * \author Emery Hemingway
 * \date   2016-07-08
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _RETRO_FRONTEND__INPUT_H_
#define _RETRO_FRONTEND__INPUT_H_

/* Genode includes */
#include <input_session/connection.h>
#include <input/event_queue.h>

namespace Retro_frontend {
#include <libretro.h>

	struct Controller;

	static unsigned genode_map[Input::Keycode::KEY_MAX];

}

struct Retro_frontend::Controller
{
	enum {
		RETRO_JOYPAD_MAX = RETRO_DEVICE_ID_JOYPAD_R3+1,
		RETRO_MOUSE_MAX = RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN
	};

	int16_t   joypad_state[RETRO_JOYPAD_MAX];
	int16_t    mouse_state[RETRO_MOUSE_MAX];
	int16_t keyboard_state[RETROK_LAST];

	Input::Connection input;

	Genode::Attached_dataspace input_ds;

	Input::Event const * const events =
		input_ds.local_addr<Input::Event>();

	bool paused = false;

	Controller(Genode::Env &env)
	: input(env), input_ds(env.rm(), input.dataspace())
	{
		Genode::memset(  joypad_state, 0x00, sizeof(  joypad_state));
		Genode::memset(   mouse_state, 0x00, sizeof(   mouse_state));
		Genode::memset(keyboard_state, 0x00, sizeof(keyboard_state));
	}

	bool unpaused()
	{
		using namespace Input;

		unsigned num_events = input.flush();

		for (unsigned i = 0; i < num_events; ++i) {
			Input::Event const &ev = events[i];
			if ((ev.code() == KEY_PAUSE) && (ev.type() == Event::Type::RELEASE)) {
				paused = false;
				return true;
			}
		}
		return false;
	}

	void flush()
	{
		using namespace Input;

		mouse_state[RETRO_DEVICE_ID_MOUSE_X] =
		mouse_state[RETRO_DEVICE_ID_MOUSE_Y] = 0;

		unsigned num_events = input.flush();
		for (unsigned i = 0; i < num_events; ++i) {
			Input::Event const &ev = events[i];

			uint16_t v = 0; /* assume a release */

			switch (ev.type()) {
			case Event::Type::MOTION:
				if (ev.relative_motion()) {
					mouse_state[RETRO_DEVICE_ID_MOUSE_X] += ev.rx();
					mouse_state[RETRO_DEVICE_ID_MOUSE_Y] += ev.ry();
				} else if (ev.absolute_motion()) {
					mouse_state[RETRO_DEVICE_ID_MOUSE_X] += ev.ax();
					mouse_state[RETRO_DEVICE_ID_MOUSE_Y] += ev.ay();
				} break;

			case Event::Type::PRESS:
				v = 1; /* not a release */
			case Event::Type::RELEASE:
				switch (ev.code()) {
				case BTN_MIDDLE:    mouse_state[RETRO_DEVICE_ID_MOUSE_MIDDLE]    = v; break;
				case BTN_GEAR_DOWN: mouse_state[RETRO_DEVICE_ID_MOUSE_WHEELDOWN] = v; break;
				case BTN_GEAR_UP:   mouse_state[RETRO_DEVICE_ID_MOUSE_WHEELUP]   = v; break;

				case BTN_B:         joypad_state[RETRO_DEVICE_ID_JOYPAD_B]       = v; break;
				case BTN_Y:         joypad_state[RETRO_DEVICE_ID_JOYPAD_Y]       = v; break;
				case BTN_SELECT:    joypad_state[RETRO_DEVICE_ID_JOYPAD_SELECT]  = v; break;
				case BTN_START:     joypad_state[RETRO_DEVICE_ID_JOYPAD_START]   = v; break;
				case BTN_FORWARD:   joypad_state[RETRO_DEVICE_ID_JOYPAD_UP]      = v; break;
				case BTN_BACK:      joypad_state[RETRO_DEVICE_ID_JOYPAD_DOWN]    = v; break;
				case BTN_LEFT:      joypad_state[RETRO_DEVICE_ID_JOYPAD_LEFT]    =
				                     mouse_state[RETRO_DEVICE_ID_MOUSE_LEFT]     = v; break;
				case BTN_RIGHT:     joypad_state[RETRO_DEVICE_ID_JOYPAD_RIGHT]   =
				                     mouse_state[RETRO_DEVICE_ID_MOUSE_RIGHT]    = v; break;
				case BTN_A:         joypad_state[RETRO_DEVICE_ID_JOYPAD_A]       = v; break;
				case BTN_X:         joypad_state[RETRO_DEVICE_ID_JOYPAD_X]       = v; break;
				case BTN_TL:        joypad_state[RETRO_DEVICE_ID_JOYPAD_L]       = v; break;
				case BTN_TR:        joypad_state[RETRO_DEVICE_ID_JOYPAD_R]       = v; break;
				case BTN_TL2:       joypad_state[RETRO_DEVICE_ID_JOYPAD_L2]      = v; break;
				case BTN_TR2:       joypad_state[RETRO_DEVICE_ID_JOYPAD_R2]      = v; break;
				case BTN_THUMBL:    joypad_state[RETRO_DEVICE_ID_JOYPAD_L3]      = v; break;
				case BTN_THUMBR:    joypad_state[RETRO_DEVICE_ID_JOYPAD_R3]      = v; break;

				case KEY_PAUSE:      paused = true; return; /* stop handling events */

				default:
					keyboard_state[genode_map[ev.code()]] = v; break;
				} break;

		/*
			case Event::Type::FOCUS:
				if (ev.code() == 0) {
					paused = true;
					return;
				}
		 */

			default: break;
			}
		}
	}

	int16_t event(unsigned device, unsigned index, unsigned id)
	{
		switch (device) {
		case RETRO_DEVICE_JOYPAD:   return   joypad_state[id];
		case RETRO_DEVICE_MOUSE:    return    mouse_state[id];
		case RETRO_DEVICE_KEYBOARD: return keyboard_state[id];
		default: return 0;
		}
	}
};


namespace Retro_frontend {

	void init_keyboard_map()
	{
		using namespace Input;
		for (unsigned i = 0; i < Input::Keycode::KEY_MAX; ++i) switch (i) {
			case KEY_ESC:         genode_map[i] = RETROK_ESCAPE; break;
			case KEY_1:           genode_map[i] = RETROK_1; break;
			case KEY_2:           genode_map[i] = RETROK_2; break;
			case KEY_3:           genode_map[i] = RETROK_3; break;
			case KEY_4:           genode_map[i] = RETROK_4; break;
			case KEY_5:           genode_map[i] = RETROK_5; break;
			case KEY_6:           genode_map[i] = RETROK_6; break;
			case KEY_7:           genode_map[i] = RETROK_7; break;
			case KEY_8:           genode_map[i] = RETROK_8; break;
			case KEY_9:           genode_map[i] = RETROK_9; break;
			case KEY_0:           genode_map[i] = RETROK_0; break;
			case KEY_MINUS:       genode_map[i] = RETROK_MINUS; break;
			case KEY_EQUAL:       genode_map[i] = RETROK_EQUALS; break;
			case KEY_BACKSPACE:   genode_map[i] = RETROK_BACKSPACE; break;
			case KEY_TAB:         genode_map[i] = RETROK_TAB; break;
			case KEY_Q:           genode_map[i] = RETROK_q; break;
			case KEY_W:           genode_map[i] = RETROK_w; break;
			case KEY_E:           genode_map[i] = RETROK_e; break;
			case KEY_R:           genode_map[i] = RETROK_r; break;
			case KEY_T:           genode_map[i] = RETROK_t; break;
			case KEY_Y:           genode_map[i] = RETROK_y; break;
			case KEY_U:           genode_map[i] = RETROK_u; break;
			case KEY_I:           genode_map[i] = RETROK_i; break;
			case KEY_O:           genode_map[i] = RETROK_o; break;
			case KEY_P:           genode_map[i] = RETROK_p; break;
			case KEY_LEFTBRACE:   genode_map[i] = RETROK_LEFTBRACKET; break;
			case KEY_RIGHTBRACE:  genode_map[i] = RETROK_RIGHTBRACKET; break;
			case KEY_ENTER:       genode_map[i] = RETROK_RETURN; break;
			case KEY_LEFTCTRL:    genode_map[i] = RETROK_LCTRL; break;
			case KEY_A:           genode_map[i] = RETROK_a; break;
			case KEY_S:           genode_map[i] = RETROK_s; break;
			case KEY_D:           genode_map[i] = RETROK_d; break;
			case KEY_F:           genode_map[i] = RETROK_f; break;
			case KEY_G:           genode_map[i] = RETROK_g; break;
			case KEY_H:           genode_map[i] = RETROK_h; break;
			case KEY_J:           genode_map[i] = RETROK_j; break;
			case KEY_K:           genode_map[i] = RETROK_k; break;
			case KEY_L:           genode_map[i] = RETROK_l; break;
			case KEY_SEMICOLON:   genode_map[i] = RETROK_SEMICOLON; break;
			case KEY_APOSTROPHE:  genode_map[i] = RETROK_QUOTE; break;
			case KEY_GRAVE:       genode_map[i] = RETROK_BACKQUOTE; break;
			case KEY_LEFTSHIFT:   genode_map[i] = RETROK_LSHIFT; break;
			case KEY_BACKSLASH:   genode_map[i] = RETROK_BACKSLASH; break;
			case KEY_Z:           genode_map[i] = RETROK_z; break;
			case KEY_X:           genode_map[i] = RETROK_x; break;
			case KEY_C:           genode_map[i] = RETROK_c; break;
			case KEY_V:           genode_map[i] = RETROK_v; break;
			case KEY_B:           genode_map[i] = RETROK_b; break;
			case KEY_N:           genode_map[i] = RETROK_n; break;
			case KEY_M:           genode_map[i] = RETROK_m; break;
			case KEY_COMMA:       genode_map[i] = RETROK_COMMA; break;
			case KEY_DOT:         genode_map[i] = RETROK_PERIOD; break;
			case KEY_SLASH:       genode_map[i] = RETROK_SLASH; break;
			case KEY_RIGHTSHIFT:  genode_map[i] = RETROK_RSHIFT; break;
			case KEY_KPASTERISK:  genode_map[i] = RETROK_KP_MULTIPLY; break;
			case KEY_LEFTALT:     genode_map[i] = RETROK_LALT; break;
			case KEY_SPACE:       genode_map[i] = RETROK_SPACE; break;
			case KEY_CAPSLOCK:    genode_map[i] = RETROK_CAPSLOCK; break;
			case KEY_F1:          genode_map[i] = RETROK_F1; break;
			case KEY_F2:          genode_map[i] = RETROK_F2; break;
			case KEY_F3:          genode_map[i] = RETROK_F3; break;
			case KEY_F4:          genode_map[i] = RETROK_F4; break;
			case KEY_F5:          genode_map[i] = RETROK_F5; break;
			case KEY_F6:          genode_map[i] = RETROK_F6; break;
			case KEY_F7:          genode_map[i] = RETROK_F7; break;
			case KEY_F8:          genode_map[i] = RETROK_F8; break;
			case KEY_F9:          genode_map[i] = RETROK_F9; break;
			case KEY_F10:         genode_map[i] = RETROK_F10; break;
			case KEY_NUMLOCK:     genode_map[i] = RETROK_NUMLOCK; break;
			case KEY_SCROLLLOCK:  genode_map[i] = RETROK_SCROLLOCK; break;
			case KEY_KP7:         genode_map[i] = RETROK_KP7; break;
			case KEY_KP8:         genode_map[i] = RETROK_KP8; break;
			case KEY_KP9:         genode_map[i] = RETROK_KP9; break;
			case KEY_KPMINUS:     genode_map[i] = RETROK_KP_MINUS; break;
			case KEY_KP4:         genode_map[i] = RETROK_KP4; break;
			case KEY_KP5:         genode_map[i] = RETROK_KP5; break;
			case KEY_KP6:         genode_map[i] = RETROK_KP6; break;
			case KEY_KPPLUS:      genode_map[i] = RETROK_KP_PLUS; break;
			case KEY_KP1:         genode_map[i] = RETROK_KP1; break;
			case KEY_KP2:         genode_map[i] = RETROK_KP2; break;
			case KEY_KP3:         genode_map[i] = RETROK_KP3; break;
			case KEY_KP0:         genode_map[i] = RETROK_KP0; break;
			case KEY_KPDOT:       genode_map[i] = RETROK_KP_PERIOD; break;

			case KEY_F11:         genode_map[i] = RETROK_F11; break;
			case KEY_F12:         genode_map[i] = RETROK_F12; break;
			case KEY_KPENTER:     genode_map[i] = RETROK_KP_ENTER; break;
			case KEY_RIGHTCTRL:   genode_map[i] = RETROK_RCTRL; break;
			case KEY_KPSLASH:     genode_map[i] = RETROK_KP_DIVIDE; break;
			case KEY_SYSRQ:       genode_map[i] = RETROK_SYSREQ; break;
			case KEY_RIGHTALT:    genode_map[i] = RETROK_RALT; break;
			case KEY_HOME:        genode_map[i] = RETROK_HOME; break;
			case KEY_UP:          genode_map[i] = RETROK_UP; break;
			case KEY_PAGEUP:      genode_map[i] = RETROK_PAGEUP; break;
			case KEY_LEFT:        genode_map[i] = RETROK_LEFT; break;
			case KEY_RIGHT:       genode_map[i] = RETROK_RIGHT; break;
			case KEY_END:         genode_map[i] = RETROK_END; break;
			case KEY_DOWN:        genode_map[i] = RETROK_DOWN; break;
			case KEY_PAGEDOWN:    genode_map[i] = RETROK_PAGEDOWN; break;
			case KEY_INSERT:      genode_map[i] = RETROK_INSERT; break;
			case KEY_DELETE:      genode_map[i] = RETROK_DELETE; break;
			case KEY_POWER:       genode_map[i] = RETROK_POWER; break;
			case KEY_KPEQUAL:     genode_map[i] = RETROK_KP_EQUALS; break;
			case KEY_KPPLUSMINUS: genode_map[i] = RETROK_KP_MINUS; break;

			case KEY_LEFTMETA:    genode_map[i] = RETROK_LMETA; break;
			case KEY_RIGHTMETA:   genode_map[i] = RETROK_RMETA; break;
			case KEY_COMPOSE:     genode_map[i] = RETROK_COMPOSE; break;

			case KEY_UNDO:        genode_map[i] = RETROK_UNDO; break;
			case KEY_HELP:        genode_map[i] = RETROK_HELP; break;
			case KEY_MENU:        genode_map[i] = RETROK_MENU; break;

			case KEY_F13:         genode_map[i] = RETROK_F13; break;
			case KEY_F14:         genode_map[i] = RETROK_F14; break;
			case KEY_F15:         genode_map[i] = RETROK_F15; break;

			case KEY_BREAK:       genode_map[i] = RETROK_BREAK; break;

			default: genode_map[i] = RETROK_UNKNOWN; break;
		}
	}

};

#endif
