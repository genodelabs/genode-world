/*
 * \brief  Input event remapper
 * \author Emery Hemingway
 * \date   2016-07-10
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <input/keycodes.h>
#include <input/component.h>
#include <input_session/connection.h>
#include <os/static_root.h>
#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <base/log.h>

namespace Input { struct Remap; }

struct Input::Remap
{
	typedef Genode::String<32> Keyname;

	int code_map[Input::Keycode::KEY_MAX];

	Genode::Env &env;

	Genode::Attached_rom_dataspace config_rom { env, "config" };

	/*
	 * Input session provided by our parent
	 */
	Input::Connection parent_input { env };

	/*
	 * Input session provided to our client
	 */
	Input::Session_component input_session_component;

	/*
	 * Attach root interface to the entry point
	 */
	Genode::Static_root<Input::Session> input_root
		{ env.ep().manage(input_session_component) };

	void event_flush()
	{
		parent_input.for_each_event([&] (Event const &e) {
			if ((e.type() == Event::PRESS) || (e.type() == Event::RELEASE))
				input_session_component.submit(Event(
					e.type(), code_map[e.code()], e.ax(), e.ay(), e.rx(), e.ry()));
			else
				input_session_component.submit(e);
		});
	}

	Genode::Signal_handler<Remap> event_flusher
		{ env.ep(), *this, &Remap::event_flush };

	static int lookup_code(Keyname const &name)
	{
		/* not the fastest way to do this, just the most terse */
		for (int code = 0; code < Input::Keycode::KEY_MAX; ++code)
			if (name == key_name((Keycode)code)) return code;
		return KEY_UNKNOWN;
	}

	void remap()
	{
		using namespace Genode;

		/* load the default mappings */
		for (int code = 0; code < Input::Keycode::KEY_MAX; ++code)
			code_map[code] = code;

		config_rom.xml().for_each_sub_node("map", [&] (Xml_node node) {
			Keyname const from = node.attribute_value("from", Keyname());
			Keyname const   to = node.attribute_value("to", Keyname());

			if ((from == "") || (to == "")) {
				char tmp[128];
				strncpy(tmp, node.addr(), min(sizeof(tmp), node.size()+1));
				error("ignoring mapping '", Cstring(tmp), "'");
				return;
			}

			int from_code = lookup_code(from);
			int   to_code = lookup_code(to);

			if (from_code == KEY_UNKNOWN) {
				error("unknown key ", from.string());
				return;
			}
			if (to_code == KEY_UNKNOWN) {
				error("unknown key ", to.string());
				return;
			}

			code_map[from_code] = to_code;
		});
	}

	Genode::Signal_handler<Remap> config_handler
		{ env.ep(), *this, &Remap::remap };

	/**
	 * Constructor
	 */
	Remap(Genode::Env &env) : env(env)
	{
		config_rom.sigh(config_handler);
		parent_input.sigh(event_flusher);

		remap();

		input_session_component.event_queue().enabled(true);

		env.parent().announce(env.ep().manage(input_root));
	}
};


/***************
 ** Component **
 ***************/

Genode::size_t Component::stack_size() { return 4*1024*sizeof(Genode::addr_t); }

void Component::construct(Genode::Env &env) { static Input::Remap inst(env); }
