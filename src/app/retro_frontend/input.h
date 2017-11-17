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

/* local includes */
#include "core.h"

/* Genode includes */
#include <input_session/connection.h>
#include <input/event_queue.h>

#include "input_maps.h"

namespace Retro_frontend {

#include <libretro.h>

const retro_controller_info *controller_info = nullptr;

/* callback to feed keyboard events to core */
static retro_keyboard_event_t keyboard_callback;

/* Genode keyboard mapped to Libretro keyboard */
static retro_key key_map[Input::Keycode::KEY_MAX];

/* array of currently configured controllers */
enum { PORT_MAX = 7 };
struct Controller;
Controller *controllers[PORT_MAX+1] = { nullptr, };

typedef Genode::String<32> Keyname;

int lookup_genode_code(Keyname const &name)
{
	using namespace Input;

	/* not the fastest way to do this, just the most terse */
	for (int code = 0; code < Input::Keycode::KEY_MAX; ++code)
		if (name == key_name((Input::Keycode)code)) return code;
	return Input::KEY_UNKNOWN;
}

}

struct Retro_frontend::Controller
{
	bool input_state[Input::Keycode::KEY_MAX];

	enum { JOYPAD_MAP_LEN = RETRO_DEVICE_ID_JOYPAD_R3 + 1 };
	int16_t joypad_map[JOYPAD_MAP_LEN];

	enum { MOUSE_MAP_LEN = RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN + 1 };
	int16_t mouse_map[MOUSE_MAP_LEN];
	int16_t mouse_x = 0;
	int16_t mouse_y = 0;

	/* TODO: analog axes */

	Input::Connection input_conn;

	Genode::Attached_dataspace input_ds;

	Input::Event const * const events =
		input_ds.local_addr<Input::Event>();

	uint16_t keymods = RETROKMOD_NONE;

	/**
	 * Use the 'label' attribute on an input configuration as a session
	 * label with a fallback to the port number.
	 */
	typedef Genode::String<Genode::Session_label::capacity()> Label;
	static Label port_label(Genode::Xml_node const &config)
	{
		if (config.has_attribute("label"))
			return config.attribute_value("label", Label());
		else
			return config.attribute_value("port", Label());
	}

	Controller(Genode::Xml_node const &config, unsigned port)
	: input_conn(*genv, port_label(config).string()),
	  input_ds(genv->rm(), input_conn.dataspace())
	{
		using namespace Input;

		Genode::memset(input_state, 0x00, sizeof(input_state));

		unsigned device = config.attribute_value("device", 0UL);
		unsigned device_class = RETRO_DEVICE_MASK & device;

		if (controller_info) {
			/* validate that the controller conforms to the core expectations */
			bool valid = false;
			for (const retro_controller_info *info = controller_info;
			     (info && (info->types)); ++info)
			{
				for (unsigned i = 0; i < info->num_types; ++i) {
					const retro_controller_description *type = &info->types[i];
					if (type->id == device) {
						valid = true;
						break;
					}
				}
			}
			if (valid)
				retro_set_controller_port_device(port, device);
			else
				Genode::error("controller ", port, " device ", device, " is not valid for core");
		}

		/* set default joypad mapping */
		joypad_map[RETRO_DEVICE_ID_JOYPAD_B] = Keycode::BTN_B;
		joypad_map[RETRO_DEVICE_ID_JOYPAD_Y] = Keycode::BTN_Y;
		joypad_map[RETRO_DEVICE_ID_JOYPAD_SELECT] = Keycode::BTN_SELECT;
		joypad_map[RETRO_DEVICE_ID_JOYPAD_START] = Keycode::BTN_START;
		joypad_map[RETRO_DEVICE_ID_JOYPAD_UP] = Keycode::BTN_FORWARD;
		joypad_map[RETRO_DEVICE_ID_JOYPAD_DOWN] = Keycode::BTN_BACK;
		joypad_map[RETRO_DEVICE_ID_JOYPAD_LEFT] = Keycode::BTN_LEFT;
		joypad_map[RETRO_DEVICE_ID_JOYPAD_RIGHT] = Keycode::BTN_RIGHT;
		joypad_map[RETRO_DEVICE_ID_JOYPAD_A] = Keycode::BTN_A;
		joypad_map[RETRO_DEVICE_ID_JOYPAD_X] = Keycode::BTN_X;
		joypad_map[RETRO_DEVICE_ID_JOYPAD_L] = Keycode::BTN_TL;
		joypad_map[RETRO_DEVICE_ID_JOYPAD_R] = Keycode::BTN_TR;
		joypad_map[RETRO_DEVICE_ID_JOYPAD_L2] = Keycode::BTN_TL2;
		joypad_map[RETRO_DEVICE_ID_JOYPAD_R2] = Keycode::BTN_TR2;
		joypad_map[RETRO_DEVICE_ID_JOYPAD_L3] = Keycode::BTN_THUMBL;
		joypad_map[RETRO_DEVICE_ID_JOYPAD_R3] = Keycode::BTN_THUMBR;

		/* set default mouse mapping */;
		mouse_map[RETRO_DEVICE_ID_MOUSE_LEFT] = Keycode::BTN_LEFT;
		mouse_map[RETRO_DEVICE_ID_MOUSE_RIGHT] = Keycode::BTN_RIGHT;
		mouse_map[RETRO_DEVICE_ID_MOUSE_WHEELUP] = Keycode::BTN_GEAR_UP;
		mouse_map[RETRO_DEVICE_ID_MOUSE_WHEELDOWN] = Keycode::BTN_GEAR_DOWN;
		mouse_map[RETRO_DEVICE_ID_MOUSE_MIDDLE] = Keycode::BTN_MIDDLE;

		auto map_fn = [&] (Genode::Xml_node map_node) {
			using namespace Genode;
			using namespace Input;

			Keyname const from = map_node.attribute_value("from", Keyname());
			Keyname const   to = map_node.attribute_value("to", Keyname());

			if ((from == "") || (to == "")) {
				error("ignoring ", map_node);
				return;
			}

			int from_code = lookup_genode_code(from);
			int   to_code = lookup_input_code(device_class, to.string());

			if (from_code == KEY_UNKNOWN) {
				error("unknown key ", from.string());
				return;
			}
			if (to_code == RETROK_LAST) {
				error("unknown key ", to.string());
				return;
			}

			/*
			 * to_code and from_code are mapped reversed from the
			 * frontend and oriented from the core perspective
			 */
			switch (device_class) {
			case RETRO_DEVICE_JOYPAD:
				if (to_code < JOYPAD_MAP_LEN) {
					joypad_map[to_code] = from_code;
				} else {
					Genode::error(to, " is not mapped for joypads");
				}
				break;
			case RETRO_DEVICE_MOUSE:
				if (to_code < MOUSE_MAP_LEN) {
					mouse_map[to_code] = from_code;
				} else {
					Genode::error(to, " is not mapped for mice");
				}

				break;
			default:
				Genode::error(map_node, " is invalid for device ",
				              device, " class ", device_class);
				break;
			}
		};

		config.for_each_sub_node("map", map_fn);
	}

	/* TODO: analog axes */

	void poll()
	{
		using namespace Input;

		unsigned num_events = input_conn.flush();
		for (unsigned i = 0; i < num_events; ++i) {
			Input::Event const &ev = events[i];

			bool v = 0; /* assume a release */

			switch (ev.type()) {

			case Event::Type::MOTION:
				/* TODO */
				continue;

			case Event::Type::PRESS:
				v = 1; /* not a release */

			case Event::Type::RELEASE: {
				int code = ev.code();
				if (code >= 0 && code < Input::KEY_MAX) {
					retro_mod mod = RETROKMOD_NONE;

					switch (code) {
					case Input::KEY_PAUSE:
						if (!v) /* pause/unpause on key release */
							toggle_pause();
						break;

					case Input::KEY_LEFTSHIFT:
					case Input::KEY_RIGHTSHIFT:
						mod = RETROKMOD_SHIFT; break;

					case Input::KEY_LEFTCTRL:
					case Input::KEY_RIGHTCTRL:
						mod = RETROKMOD_CTRL; break;

					case Input::KEY_LEFTALT:
					case Input::KEY_RIGHTALT:
						mod = RETROKMOD_ALT; break;

					case Input::KEY_LEFTMETA:
					case Input::KEY_RIGHTMETA:
						mod = RETROKMOD_META; break;

					case Input::KEY_NUMLOCK:
						mod = RETROKMOD_NUMLOCK; break;

					case Input::KEY_CAPSLOCK:
						mod = RETROKMOD_CAPSLOCK; break;

					case Input::KEY_SCROLLLOCK:
						mod = RETROKMOD_SCROLLOCK; break;

					default: break;
					}

					if (mod)
						keymods = (keymods | mod) - (v ? 0 : mod);

					input_state[code] = v;

					if (keyboard_callback) {
						/*
						 * the keyboard_callback may drive the core
						 * but this context is a libretro callback
						 * so 'with_libc' must be in effect
						 */
						unsigned rk = key_map[code];
						if (rk != RETROK_UNKNOWN) {
							keyboard_callback(v, rk, 0, keymods);
						}
					}
				}
			}

			case Event::Type::CHARACTER:
				if (keyboard_callback) {
					/* event only supports UTF-8? */
					keyboard_callback(v, RETROK_UNKNOWN, ev.utf8().b0, RETROKMOD_NONE);
				}
				break;

			default: break;
			}
		}
	}
};


namespace Retro_frontend {

void initialize_controllers(Genode::Allocator &alloc,
                            Genode::Xml_node config)
{
	using namespace Genode;

	Genode::memset(key_map, 0x00, sizeof(key_map));
	for (int i = 0; default_keymap[i].gk < Input::KEY_MAX; ++i) {
		key_map[default_keymap[i].gk] = (retro_key)default_keymap[i].rk;
	}

	for (int i = 0; i <= PORT_MAX; ++i) {
		if (controllers[i] != nullptr) {
			destroy(alloc, controllers[i]);
			controllers[i] = nullptr;
			retro_set_controller_port_device(i, RETRO_DEVICE_NONE);
		}
	}

	config.for_each_sub_node("controller", [&] (Xml_node const &input_node) {
		unsigned port = input_node.attribute_value("port", 0U);
		unsigned device = input_node.attribute_value("device", 0U);

		if (controllers[port] != nullptr) {
			error("controller port ", port, " is already configured");
			return;
		}

		controllers[port] = new (alloc) Controller(input_node, port);
		if (device != RETRO_DEVICE_NONE)
			retro_set_controller_port_device(port, device);
	});
}


void input_poll_callback()
{
	for (int i = 0; i <= PORT_MAX; ++i) {
		if (controllers[i])
			controllers[i]->poll();
	}
}


int16_t input_state_callback(unsigned port,  unsigned device,
                             unsigned index, unsigned id)
{
	Controller *ctrl = (port < PORT_MAX) ? controllers[port] : 0;

	if (ctrl) switch (RETRO_DEVICE_MASK & device) {
	case RETRO_DEVICE_JOYPAD:
		if (id < Controller::JOYPAD_MAP_LEN) {
			auto code = ctrl->joypad_map[id];
			if (code < Input::KEY_MAX) {
				return ctrl->input_state[code];
			}
		}
		break;
	case RETRO_DEVICE_MOUSE:
		switch (id) {
		case RETRO_DEVICE_ID_MOUSE_X:
			return ctrl->mouse_x;
		case RETRO_DEVICE_ID_MOUSE_Y:
			return ctrl->mouse_y;
		default:
			if (id < Controller::MOUSE_MAP_LEN) {
				auto code = ctrl->mouse_map[id];
				if (code < Input::KEY_MAX) {
					return ctrl->input_state[code];
				}
			}
			break;
		}
		break;
	default: break;
	}

	return 0;
}

/* install a handler to wake the frontend from input events */
void input_sigh(Genode::Signal_context_capability sig)
{
	for (int i = 0; i <= PORT_MAX; ++i) {
		if (controllers[i])
			controllers[i]->input_conn.sigh(sig);
	}
}

};

#endif
