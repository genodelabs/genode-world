/*
 * \brief  Manager of all VM requested console functionality
 * \author Markus Partheymueller
 * \date   2012-07-31
 */

/*
 * Copyright (C) 2012 Intel Corporation
 * Copyright (C) 2013-2017 Genode Labs GmbH
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
#include "synced_motherboard.h"
#include "guest_memory.h"
#include "gui.h"

namespace Seoul {
	class Console;
	using Genode::Pixel_rgb888;
	using Genode::Dataspace_capability;
}

class Seoul::Console : public StaticReceiver<Seoul::Console>
{
	private:

		Genode::Env                  &_env;
		Motherboard                  &_unsynchronized_motherboard;
		Synced_motherboard           &_motherboard;

		Genode::Signal_handler<Console> _signal_input
			= { _env.ep(), *this, &Console::_handle_input };

		Genode::List<Backend_gui>     _guis { };
		Genode::Allocator            &_alloc;

		Backend_gui                  &_backend_gui;
		Seoul::Guest_memory          &_memory;
		Framebuffer::Mode    const    _fb_mode;
		size_t               const    _fb_size;
		Dataspace_capability const    _fb_vm_ds;
		Genode::addr_t       const    _fb_vm_mapping;
		Genode::addr_t       const    _vm_phys_fb;
		short                        *_pixels;
		unsigned                      _timer    { 0 };
		Keyboard                      _vkeyb    { _motherboard };
		char                         *_guest_fb { nullptr };
		VgaRegs                      *_regs     { nullptr };
		bool                          _left     { false };
		bool                          _middle   { false };
		bool                          _right    { false };
		bool                          _relative { false };
		bool                          _absolute { false };

		unsigned _input_to_ps2mouse(Input::Event const &);
		unsigned _input_to_ps2wheel(Input::Event const &);
		void     _input_to_virtio(Input::Event const &);
		void     _input_to_ps2(Input::Event const &);

		void _handle_input();
		unsigned _handle_fb(bool);

		void _reactivate();

		/*
		 * Noncopyable
		 */
		Console(Console const &);
		Console &operator = (Console const &);

	public:

		enum { ID_VGA_VESA = 0 };

		Genode::addr_t attached_framebuffer() const { return _fb_vm_mapping; }
		Genode::addr_t framebuffer_size()     const { return _fb_size; }
		Genode::addr_t vm_phys_framebuffer()  const { return _vm_phys_fb; }

		/* bus callbacks */
		bool receive(MessageConsole &);
		bool receive(MessageMemRegion &);
		bool receive(MessageTimeout &);

		void register_host_operations(Motherboard &);

		/**
		 * Constructor
		 */
		Console(Genode::Env &env, Genode::Allocator &alloc,
		        Synced_motherboard &, Motherboard &,
		        Gui::Area const, Seoul::Guest_memory &);

		template <typename FUNC>
		bool apply_msg(unsigned const id, FUNC const& fn) {
			for (auto *gui = _guis.first(); gui; gui = gui->next()) {
				if (gui->id == id) {
					return fn(*gui);
				}
			}
			return false;
		}
};

#endif /* _CONSOLE_H_ */
