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

#ifndef _RETRO_FRONTEND__INPUT_MAPS_H_
#define _RETRO_FRONTEND__INPUT_MAPS_H_

/* Genode includes */
#include <input/keycodes.h>

namespace Retro_frontend {
#include <libretro.h>

struct retro_input_text_map
{
   int code;
	char const *text;
};

static const struct retro_input_text_map joypad_text_map[] = {
	{ RETRO_DEVICE_ID_JOYPAD_B, "B" },
	{ RETRO_DEVICE_ID_JOYPAD_Y, "Y" },
	{ RETRO_DEVICE_ID_JOYPAD_SELECT, "SELECT" },
	{ RETRO_DEVICE_ID_JOYPAD_START, "START" },
	{ RETRO_DEVICE_ID_JOYPAD_UP, "UP" },
	{ RETRO_DEVICE_ID_JOYPAD_DOWN, "DOWN" },
	{ RETRO_DEVICE_ID_JOYPAD_LEFT, "LEFT" },
	{ RETRO_DEVICE_ID_JOYPAD_RIGHT, "RIGHT" },
	{ RETRO_DEVICE_ID_JOYPAD_A, "A" },
	{ RETRO_DEVICE_ID_JOYPAD_X, "X" },
	{ RETRO_DEVICE_ID_JOYPAD_L, "L" },
	{ RETRO_DEVICE_ID_JOYPAD_R, "R" },
	{ RETRO_DEVICE_ID_JOYPAD_L2, "L2" },
	{ RETRO_DEVICE_ID_JOYPAD_R2, "R2" },
	{ RETRO_DEVICE_ID_JOYPAD_L3, "L3" },
	{ RETRO_DEVICE_ID_JOYPAD_R3, "R3" },
	{ -1, "" },
};

static const struct retro_input_text_map mouse_text_map[] = {
	{ RETRO_DEVICE_ID_MOUSE_LEFT, "LEFT" },
	{ RETRO_DEVICE_ID_MOUSE_RIGHT, "RIGHT" },
	{ RETRO_DEVICE_ID_MOUSE_WHEELUP, "WHEELUP" },
	{ RETRO_DEVICE_ID_MOUSE_WHEELDOWN, "WHEELDOWN" },
	{ RETRO_DEVICE_ID_MOUSE_MIDDLE, "MIDDLE" },
	{ RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELUP, "HORIZ_WHEELUP" },
	{ RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN, "HORIZ_WHEELDOWN" },
	{ -1, "" },
};

static const struct retro_input_text_map keyboard_text_map[] = {
	{ RETROK_UNKNOWN, "UNKNOWN" },
	{ RETROK_FIRST, "FIRST" },
	{ RETROK_BACKSPACE, "BACKSPACE" },
	{ RETROK_TAB, "TAB" },
	{ RETROK_CLEAR, "CLEAR" },
	{ RETROK_RETURN, "RETURN" },
	{ RETROK_PAUSE, "PAUSE" },
	{ RETROK_ESCAPE, "ESCAPE" },
	{ RETROK_SPACE, "SPACE" },
	{ RETROK_EXCLAIM, "EXCLAIM" },
	{ RETROK_QUOTEDBL, "QUOTEDBL" },
	{ RETROK_HASH, "HASH" },
	{ RETROK_DOLLAR, "DOLLAR" },
	{ RETROK_AMPERSAND, "AMPERSAND" },
	{ RETROK_QUOTE, "QUOTE" },
	{ RETROK_LEFTPAREN, "LEFTPAREN" },
	{ RETROK_RIGHTPAREN, "RIGHTPAREN" },
	{ RETROK_ASTERISK, "ASTERISK" },
	{ RETROK_PLUS, "PLUS" },
	{ RETROK_COMMA, "COMMA" },
	{ RETROK_MINUS, "MINUS" },
	{ RETROK_PERIOD, "PERIOD" },
	{ RETROK_SLASH, "SLASH" },
	{ RETROK_0, "0" },
	{ RETROK_1, "1" },
	{ RETROK_2, "2" },
	{ RETROK_3, "3" },
	{ RETROK_4, "4" },
	{ RETROK_5, "5" },
	{ RETROK_6, "6" },
	{ RETROK_7, "7" },
	{ RETROK_8, "8" },
	{ RETROK_9, "9" },
	{ RETROK_COLON, "COLON" },
	{ RETROK_SEMICOLON, "SEMICOLON" },
	{ RETROK_LESS, "LESS" },
	{ RETROK_EQUALS, "EQUALS" },
	{ RETROK_GREATER, "GREATER" },
	{ RETROK_QUESTION, "QUESTION" },
	{ RETROK_AT, "AT" },
	{ RETROK_LEFTBRACKET, "LEFTBRACKET" },
	{ RETROK_BACKSLASH, "BACKSLASH" },
	{ RETROK_RIGHTBRACKET, "RIGHTBRACKET" },
	{ RETROK_CARET, "CARET" },
	{ RETROK_UNDERSCORE, "UNDERSCORE" },
	{ RETROK_BACKQUOTE, "BACKQUOTE" },
	{ RETROK_a, "a" },
	{ RETROK_b, "b" },
	{ RETROK_c, "c" },
	{ RETROK_d, "d" },
	{ RETROK_e, "e" },
	{ RETROK_f, "f" },
	{ RETROK_g, "g" },
	{ RETROK_h, "h" },
	{ RETROK_i, "i" },
	{ RETROK_j, "j" },
	{ RETROK_k, "k" },
	{ RETROK_l, "l" },
	{ RETROK_m, "m" },
	{ RETROK_n, "n" },
	{ RETROK_o, "o" },
	{ RETROK_p, "p" },
	{ RETROK_q, "q" },
	{ RETROK_r, "r" },
	{ RETROK_s, "s" },
	{ RETROK_t, "t" },
	{ RETROK_u, "u" },
	{ RETROK_v, "v" },
	{ RETROK_w, "w" },
	{ RETROK_x, "x" },
	{ RETROK_y, "y" },
	{ RETROK_z, "z" },
	{ RETROK_DELETE, "DELETE" },

	{ RETROK_KP0, "KP0" },
	{ RETROK_KP1, "KP1" },
	{ RETROK_KP2, "KP2" },
	{ RETROK_KP3, "KP3" },
	{ RETROK_KP4, "KP4" },
	{ RETROK_KP5, "KP5" },
	{ RETROK_KP6, "KP6" },
	{ RETROK_KP7, "KP7" },
	{ RETROK_KP8, "KP8" },
	{ RETROK_KP9, "KP9" },
	{ RETROK_KP_PERIOD, "KP_PERIOD" },
	{ RETROK_KP_DIVIDE, "KP_DIVIDE" },
	{ RETROK_KP_MULTIPLY, "KP_MULTIPLY" },
	{ RETROK_KP_MINUS, "KP_MINUS" },
	{ RETROK_KP_PLUS, "KP_PLUS" },
	{ RETROK_KP_ENTER, "KP_ENTER" },
	{ RETROK_KP_EQUALS, "KP_EQUALS" },

	{ RETROK_UP, "UP" },
	{ RETROK_DOWN, "DOWN" },
	{ RETROK_RIGHT, "RIGHT" },
	{ RETROK_LEFT, "LEFT" },
	{ RETROK_INSERT, "INSERT" },
	{ RETROK_HOME, "HOME" },
	{ RETROK_END, "END" },
	{ RETROK_PAGEUP, "PAGEUP" },
	{ RETROK_PAGEDOWN, "PAGEDOWN" },

	{ RETROK_F1, "F1" },
	{ RETROK_F2, "F2" },
	{ RETROK_F3, "F3" },
	{ RETROK_F4, "F4" },
	{ RETROK_F5, "F5" },
	{ RETROK_F6, "F6" },
	{ RETROK_F7, "F7" },
	{ RETROK_F8, "F8" },
	{ RETROK_F9, "F9" },
	{ RETROK_F10, "F10" },
	{ RETROK_F11, "F11" },
	{ RETROK_F12, "F12" },
	{ RETROK_F13, "F13" },
	{ RETROK_F14, "F14" },
	{ RETROK_F15, "F15" },

	{ RETROK_NUMLOCK, "NUMLOCK" },
	{ RETROK_CAPSLOCK, "CAPSLOCK" },
	{ RETROK_SCROLLOCK, "SCROLLOCK" },
	{ RETROK_RSHIFT, "RSHIFT" },
	{ RETROK_LSHIFT, "LSHIFT" },
	{ RETROK_RCTRL, "RCTRL" },
	{ RETROK_LCTRL, "LCTRL" },
	{ RETROK_RALT, "RALT" },
	{ RETROK_LALT, "LALT" },
	{ RETROK_RMETA, "RMETA" },
	{ RETROK_LMETA, "LMETA" },
	{ RETROK_LSUPER, "LSUPER" },
	{ RETROK_RSUPER, "RSUPER" },
	{ RETROK_MODE, "MODE" },
	{ RETROK_COMPOSE, "COMPOSE" },

	{ RETROK_HELP, "HELP" },
	{ RETROK_PRINT, "PRINT" },
	{ RETROK_SYSREQ, "SYSREQ" },
	{ RETROK_BREAK, "BREAK" },
	{ RETROK_MENU, "MENU" },
	{ RETROK_POWER, "POWER" },
	{ RETROK_EURO, "EURO" },
	{ RETROK_UNDO, "UNDO" },
	{ -1, "" },
};

char const *lookup_input_text(int device, int code)
{
	retro_input_text_map const *map;
	char const *placeholder = "UNKNOWN";

	switch (device) {
	case RETRO_DEVICE_JOYPAD:
		map = &joypad_text_map[0]; break;
	case RETRO_DEVICE_MOUSE:
		map = &mouse_text_map[0]; break;
	case RETRO_DEVICE_KEYBOARD:
		map = &keyboard_text_map[0]; break;
	default:
		return placeholder;
	}

	for (int i = 0; *map[i].text; ++i) {
		if (map[i].code == code)
			return map[i].text;
	}
	return placeholder;
}


unsigned lookup_input_code(int device, char const *text)
{
	retro_input_text_map const *map;

	switch (device) {
	case RETRO_DEVICE_JOYPAD:
		map = &joypad_text_map[0]; break;
	case RETRO_DEVICE_MOUSE:
		map = &mouse_text_map[0]; break;
	case RETRO_DEVICE_KEYBOARD:
		map = &keyboard_text_map[0]; break;
	default: return RETROK_LAST;
	}

	for (int i = 0; *map[i].text; ++i) {
		if (Genode::strcmp(map[i].text, text) == 0)
			return map[i].code;
	}
	return RETROK_LAST;
}


struct genode_retro_map
{
   enum Input::Keycode gk;
   unsigned rk;
};

static const struct genode_retro_map default_keymap[] = {
	{ Input::KEY_ESC, RETROK_ESCAPE },
	{ Input::KEY_1, RETROK_1 },
	{ Input::KEY_2, RETROK_2 },
	{ Input::KEY_3, RETROK_3 },
	{ Input::KEY_4, RETROK_4 },
	{ Input::KEY_5, RETROK_5 },
	{ Input::KEY_6, RETROK_6 },
	{ Input::KEY_7, RETROK_7 },
	{ Input::KEY_8, RETROK_8 },
	{ Input::KEY_9, RETROK_9 },
	{ Input::KEY_0, RETROK_0 },
	{ Input::KEY_MINUS, RETROK_MINUS },
	{ Input::KEY_EQUAL, RETROK_EQUALS },
	{ Input::KEY_BACKSPACE, RETROK_BACKSPACE },
	{ Input::KEY_TAB, RETROK_TAB },
	{ Input::KEY_Q, RETROK_q },
	{ Input::KEY_W, RETROK_w },
	{ Input::KEY_E, RETROK_e },
	{ Input::KEY_R, RETROK_r },
	{ Input::KEY_T, RETROK_t },
	{ Input::KEY_Y, RETROK_y },
	{ Input::KEY_U, RETROK_u },
	{ Input::KEY_I, RETROK_i },
	{ Input::KEY_O, RETROK_o },
	{ Input::KEY_P, RETROK_p },
	{ Input::KEY_LEFTBRACE, RETROK_LEFTBRACKET },
	{ Input::KEY_RIGHTBRACE, RETROK_RIGHTBRACKET },
	{ Input::KEY_ENTER, RETROK_RETURN },
	{ Input::KEY_LEFTCTRL, RETROK_LCTRL },
	{ Input::KEY_A, RETROK_a },
	{ Input::KEY_S, RETROK_s },
	{ Input::KEY_D, RETROK_d },
	{ Input::KEY_F, RETROK_f },
	{ Input::KEY_G, RETROK_g },
	{ Input::KEY_H, RETROK_h },
	{ Input::KEY_J, RETROK_j },
	{ Input::KEY_K, RETROK_k },
	{ Input::KEY_L, RETROK_l },
	{ Input::KEY_SEMICOLON, RETROK_SEMICOLON },
	{ Input::KEY_APOSTROPHE, RETROK_QUOTE },
	{ Input::KEY_GRAVE, RETROK_BACKQUOTE },
	{ Input::KEY_LEFTSHIFT, RETROK_LSHIFT },
	{ Input::KEY_BACKSLASH, RETROK_BACKSLASH },
	{ Input::KEY_Z, RETROK_z },
	{ Input::KEY_X, RETROK_x },
	{ Input::KEY_C, RETROK_c },
	{ Input::KEY_V, RETROK_v },
	{ Input::KEY_B, RETROK_b },
	{ Input::KEY_N, RETROK_n },
	{ Input::KEY_M, RETROK_m },
	{ Input::KEY_COMMA, RETROK_COMMA },
	{ Input::KEY_DOT, RETROK_PERIOD },
	{ Input::KEY_SLASH, RETROK_SLASH },
	{ Input::KEY_RIGHTSHIFT, RETROK_RSHIFT },
	{ Input::KEY_KPASTERISK, RETROK_KP_MULTIPLY },
	{ Input::KEY_LEFTALT, RETROK_LALT },
	{ Input::KEY_SPACE, RETROK_SPACE },
	{ Input::KEY_CAPSLOCK, RETROK_CAPSLOCK },
	{ Input::KEY_F1, RETROK_F1 },
	{ Input::KEY_F2, RETROK_F2 },
	{ Input::KEY_F3, RETROK_F3 },
	{ Input::KEY_F4, RETROK_F4 },
	{ Input::KEY_F5, RETROK_F5 },
	{ Input::KEY_F6, RETROK_F6 },
	{ Input::KEY_F7, RETROK_F7 },
	{ Input::KEY_F8, RETROK_F8 },
	{ Input::KEY_F9, RETROK_F9 },
	{ Input::KEY_F10, RETROK_F10 },
	{ Input::KEY_NUMLOCK, RETROK_NUMLOCK },
	{ Input::KEY_SCROLLLOCK, RETROK_SCROLLOCK },
	{ Input::KEY_KP7, RETROK_KP7 },
	{ Input::KEY_KP8, RETROK_KP8 },
	{ Input::KEY_KP9, RETROK_KP9 },
	{ Input::KEY_KPMINUS, RETROK_KP_MINUS },
	{ Input::KEY_KP4, RETROK_KP4 },
	{ Input::KEY_KP5, RETROK_KP5 },
	{ Input::KEY_KP6, RETROK_KP6 },
	{ Input::KEY_KPPLUS, RETROK_KP_PLUS },
	{ Input::KEY_KP1, RETROK_KP1 },
	{ Input::KEY_KP2, RETROK_KP2 },
	{ Input::KEY_KP3, RETROK_KP3 },
	{ Input::KEY_KP0, RETROK_KP0 },
	{ Input::KEY_KPDOT, RETROK_KP_PERIOD },

	{ Input::KEY_F11, RETROK_F11 },
	{ Input::KEY_F12, RETROK_F12 },
	{ Input::KEY_KPENTER, RETROK_KP_ENTER },
	{ Input::KEY_RIGHTCTRL, RETROK_RCTRL },
	{ Input::KEY_KPSLASH, RETROK_KP_DIVIDE },
	{ Input::KEY_SYSRQ, RETROK_SYSREQ },
	{ Input::KEY_RIGHTALT, RETROK_RALT },
	{ Input::KEY_HOME, RETROK_HOME },
	{ Input::KEY_UP, RETROK_UP },
	{ Input::KEY_PAGEUP, RETROK_PAGEUP },
	{ Input::KEY_LEFT, RETROK_LEFT },
	{ Input::KEY_RIGHT, RETROK_RIGHT },
	{ Input::KEY_END, RETROK_END },
	{ Input::KEY_DOWN, RETROK_DOWN },
	{ Input::KEY_PAGEDOWN, RETROK_PAGEDOWN },
	{ Input::KEY_INSERT, RETROK_INSERT },
	{ Input::KEY_DELETE, RETROK_DELETE },
	{ Input::KEY_POWER, RETROK_POWER },
	{ Input::KEY_KPEQUAL, RETROK_KP_EQUALS },
	{ Input::KEY_KPPLUSMINUS, RETROK_KP_MINUS },

	{ Input::KEY_LEFTMETA, RETROK_LMETA },
	{ Input::KEY_RIGHTMETA, RETROK_RMETA },
	{ Input::KEY_COMPOSE, RETROK_COMPOSE },

	{ Input::KEY_UNDO, RETROK_UNDO },
	{ Input::KEY_HELP, RETROK_HELP },
	{ Input::KEY_MENU, RETROK_MENU },

	{ Input::KEY_F13, RETROK_F13 },
	{ Input::KEY_F14, RETROK_F14 },
	{ Input::KEY_F15, RETROK_F15 },

	{ Input::KEY_BREAK, RETROK_BREAK },
	{ Input::KEY_MAX, RETROK_LAST },
	{ Input::KEY_MAX, RETROK_LAST },
};

}

#endif