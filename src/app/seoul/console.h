/*
 * \brief  Manager of all VM requested console functionality
 * \author Markus Partheymueller
 * \date   2012-07-31
 */

/*
 * Copyright (C) 2012 Intel Corporation
 * Copyright (C) 2013-2024 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 *
 * The code is partially based on the Vancouver VMM, which is distributed
 * under the terms of the GNU General Public License version 2.
 *
 * Modifications by Intel Corporation are contributed under the terms and
 * conditions of the GNU General Public License version 2.
 */

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

/* base includes */
#include <base/env.h>
#include <base/duration.h>
#include <dataspace/client.h>
#include <util/string.h>
#include <util/bit_array.h>

/* os includes */
#include <framebuffer_session/connection.h>
#include <input/event.h>
#include <gui_session/connection.h>
#include <timer_session/connection.h>

#include <os/pixel_rgb888.h>

/* local includes */
#include "keyboard.h"
#include "guest_memory.h"
#include "gui.h"
#include "vga_vesa.h"

namespace Seoul {
	class Console;
	using Genode::Pixel_rgb888;
	using Genode::Dataspace_capability;
	using Genode::Milliseconds;
}

class Seoul::Console : public StaticReceiver<Seoul::Console>
{
	private:

		Genode::Env               &_env;
		Motherboard               &_mb;
		Genode::Mutex              _mutex { };
		Genode::List<Backend_gui>  _guis  { };
		Genode::Allocator         &_alloc;

		Seoul::Guest_memory       &_memory;
		Gui::Area  const           _gui_vesa;
		Gui::Area                  _gui_non_vesa;
		Gui::Area                  _gui_non_vesa_ack;
		Gui::Area                  _input_absolute;

		unsigned  _timer { 0 };
		Keyboard  _vkeyb { _mb };

		int       _ox = 0, _oy = 0;
		bool      _left        { };
		bool      _middle      { };
		bool      _right       { };
		bool      _relative    { };
		bool      _absolute    { };
		bool      _cpus_active { };

		Vga_vesa  _vga_vesa;

		unsigned _input_to_ps2mouse(Input::Event const &);
		unsigned _input_to_ps2wheel(Input::Event const &);
		void     _input_to_virtio  (Input::Event const &);
		void     _input_to_ps2     (Input::Event const &);

		void         _handle_input();
		void         _handle_gui_change();
		Milliseconds _handle_fb();
		unsigned     _handle_fb_gui(bool, Backend_gui &, bool);
		bool         _sufficient_ram(Gui::Area const &, Gui::Area const &);

		void _reactivate_periodic_timer();

		Genode::Signal_handler<Console> _signal_input
			= { _env.ep(), *this, &Console::_handle_input };

		Genode::Signal_handler<Console> _signal_gui
			= { _env.ep(), *this, &Console::_handle_gui_change };

		/*
		 * Noncopyable
		 */
		Console(Console const &);
		Console &operator = (Console const &);

	public:

		enum { ID_VGA_VESA = 0 };

		/* bus callbacks */
		bool receive(MessageConsole &);
		bool receive(MessageMemRegion &);
		bool receive(MessageTimeout &);

		/**
		 * Constructor
		 */
		Console(Genode::Env &, Genode::Allocator &, Motherboard &,
		        Gui::Area const, Seoul::Guest_memory &);

		bool apply_msg(unsigned const id, auto const & fn)
		{
			for (auto *gui = _guis.first(); gui; gui = gui->next()) {
				if (gui->id == id) {
					return fn(*gui);
				}
			}
			return false;
		}

		void for_each_gui(auto const & fn)
		{
			for (auto * gui = _guis.first(); gui; gui = gui->next()) {
				fn(*gui);
			}
		}
};

#endif /* _CONSOLE_H_ */
