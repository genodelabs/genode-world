/*
 * \brief  Manager of all VM requested console functionality
 * \author Markus Partheymueller
 * \author Norman Feske
 * \author Alexander Boettcher
 * \date   2012-07-31
 */

/*
 * Copyright (C) 2011-2024 Genode Labs GmbH
 * Copyright (C) 2012 Intel Corporation
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

/* base includes */
#include <util/register.h>

#include <nul/motherboard.h>
#include <host/screen.h>

/* local includes */
#include "console.h"


extern char _binary_mono_tff_start[];


/**
 * Layout of PS/2 mouse packet
 */
struct Ps2_mouse_packet : Genode::Register<32>
{
	struct Packet_size   : Bitfield<0, 3> { };
	struct Left_button   : Bitfield<8, 1> { };
	struct Middle_button : Bitfield<9, 1> { };
	struct Right_button  : Bitfield<10, 1> { };
	struct Rx_high       : Bitfield<12, 1> { };
	struct Ry_high       : Bitfield<13, 1> { };
	struct Rx_low        : Bitfield<16, 8> { };
	struct Ry_low        : Bitfield<24, 8> { };
};


static bool mouse_event(Input::Event const &ev)
{
	using namespace Input;

	bool result = false;

	auto mouse_button = [] (Keycode key) {
		return key == BTN_LEFT || key == BTN_MIDDLE || key == BTN_RIGHT; };

	ev.handle_press([&] (Keycode key, Genode::Codepoint) {
		result |= mouse_button(key); });

	ev.handle_release([&] (Keycode key) {
		result |= mouse_button(key); });

	result |= ev.absolute_motion() || ev.relative_motion();
	result |= ev.wheel();

	return result;
}


/**
 * Convert Genode::Input event to PS/2 packet
 *
 * This function updates _left, _middle, and _right as a side effect.
 */
unsigned Seoul::Console::_input_to_ps2mouse(Input::Event const &ev)
{
	/* track state of mouse buttons */
	auto apply_button = [] (Input::Event const &ev, Input::Keycode key, bool &state) {
		if (ev.key_press  (key)) state = true;
		if (ev.key_release(key)) state = false;
	};

	apply_button(ev, Input::BTN_LEFT,   _left);
	apply_button(ev, Input::BTN_MIDDLE, _middle);
	apply_button(ev, Input::BTN_RIGHT,  _right);

	int rx = 0;
	int ry = 0;

	ev.handle_absolute_motion([&] (int x, int y) {
		rx = x - _ox; ry = y - _oy;
		_ox = x; _oy = y;
	});

	ev.handle_relative_motion([&] (int x, int y) { rx = x; ry = y; });


	/* clamp relative motion vector to bounds */
	int const boundary = 200;
	rx =  Genode::min(boundary, Genode::max(-boundary, rx));
	ry = -Genode::min(boundary, Genode::max(-boundary, ry));

	/* assemble PS/2 packet */
	Ps2_mouse_packet::access_t packet = 0;
	Ps2_mouse_packet::Packet_size::set  (packet, 3);
	Ps2_mouse_packet::Left_button::set  (packet, _left);
	Ps2_mouse_packet::Middle_button::set(packet, _middle);
	Ps2_mouse_packet::Right_button::set (packet, _right);
	Ps2_mouse_packet::Rx_high::set      (packet, (rx >> 8) & 1);
	Ps2_mouse_packet::Ry_high::set      (packet, (ry >> 8) & 1);
	Ps2_mouse_packet::Rx_low::set       (packet, rx & 0xff);
	Ps2_mouse_packet::Ry_low::set       (packet, ry & 0xff);

	return packet;
}


void Seoul::Console::_input_to_ps2(Input::Event const &ev)
{
	/* if absolute pointing model is avail, don't use relative PS2 */
	if (_absolute)
		return;

	/* update PS2 mouse model */
	MessageInput msg(0x10001, _input_to_ps2mouse(ev), _input_to_ps2wheel(ev));
	if (_mb.bus_input.send(msg) && !_relative)
		_relative = true;
}


void Seoul::Console::_input_to_virtio(Input::Event const &ev)
{
	auto button = [] (Input::Event const &ev, Input::Keycode key, bool &state) {
		if (ev.key_press  (key)) {
			state = true;
			return true;
		}
		if (ev.key_release(key)) {
			state = false;
			return true;
		}
		return false;
	};

	bool left = false, middle = false, right = false;
	unsigned data = 0, data2 = 0;

	if (button(ev, Input::BTN_LEFT, left)) {
		data  = 1u << 31;
		data2 = !!left;
	} else
	if (button(ev, Input::BTN_MIDDLE, middle)) {
		data  = 1u << 30;
		data2 = !!middle;
	} else
	if (button(ev, Input::BTN_RIGHT, right)) {
		data  = 1u << 29;
		data2 = !!right;
	} else {
		ev.handle_absolute_motion([&] (int x, int y) {
			unsigned const mask = (0xfu << 28) - 1;
			unsigned tx = _input_absolute.w * x / _gui_non_vesa_ack.w;
			unsigned ty = _input_absolute.h * y / _gui_non_vesa_ack.h;

			if (x < 0 || y < 0)
				return;

			if (tx > _input_absolute.w) tx = _input_absolute.w;
			if (ty > _input_absolute.h) ty = _input_absolute.h;

			MessageInput msg(0x10002, tx & mask, ty & mask);
			if (_mb.bus_input.send(msg)) {
				_absolute = true;
				_input_absolute = Gui::Area(msg.data, msg.data2);
			}
		});
		ev.handle_wheel([&](int, int z) {
			MessageInput msg(0x10002, 1u << 28, z);
			_mb.bus_input.send(msg);
		});

		return;
	}

	MessageInput msg(0x10002, data, data2);
	_mb.bus_input.send(msg);
}


unsigned Seoul::Console::_input_to_ps2wheel(Input::Event const &ev)
{
	int rz = 0;
	ev.handle_wheel([&](int x, int y) { rz = y; });

	return (unsigned)rz;
}


bool Seoul::Console::receive(MessageConsole &msg)
{
	switch (msg.type) {
	case MessageConsole::TYPE_ALLOC_CLIENT:
	{
		bool create_new = true;

		apply_msg(msg.id, [&](auto &gui) {
			create_new = false;
			return true;
		});

		if (create_new) {
			auto const gui_area = (msg.id == ID_VGA_VESA)
			                    ? _gui_vesa : _gui_non_vesa;
			Genode::String<12> name((msg.id == ID_VGA_VESA) ?
			                        "vga/vesa" : "fb", msg.id, ".", msg.view);
			auto &gui = *new (_alloc) Backend_gui(_env, _guis, msg.id, gui_area,
			                                      _signal_input, name.string());

			if (!gui.pixels) {
				_guis.remove(&gui);
				Genode::destroy(_alloc, &gui);
				return false;
			}

			if (msg.id != ID_VGA_VESA)
				gui.gui.info_sigh(_signal_gui);
		}

		apply_msg(msg.id, [&](auto &gui) {
			/* vga_vesa re-creation is not supported */
			if (!create_new && msg.id != ID_VGA_VESA) {
				Gui::Rect const win = gui.gui_window();
				gui.resize(_env, win.area);
				_gui_non_vesa = win.area;
			}

			msg.ptr  = (char *)gui.pixels;
			msg.size = gui.fb_size();
			return !!gui.pixels;
		});

		return true;
	}
	case MessageConsole::TYPE_ALLOC_VIEW :
	{
		if (msg.id == ID_VGA_VESA) {
			apply_msg(msg.id, [&](auto &gui) {
				auto const vm_phys_addr_framebuffer = msg.size;

				_vga_vesa.init(msg.regs, msg.ptr, vm_phys_addr_framebuffer);
				_memory.add_region(_alloc, vm_phys_addr_framebuffer,
				                   gui.pixels, gui.fb_ds, gui.fb_size());

				return true;
			});
			msg.view = 0;
			return true;
		}

		bool const found = apply_msg(msg.id, [&](auto &gui) {
			if (msg.view == 1) {
				msg.ptr = gui.shape_ptr();
				msg.size = gui.shape_size();
				return true;
			}

			if (msg.view > 1 && msg.size) {
				if (Genode::align_addr(msg.size, 12) > _env.pd().avail_ram().value) {
					error("gpu memory allocation denied, requires ", msg.size,
					      ", available ", _env.pd().avail_ram(),
					      " id=", msg.view - 2);
					return false;
				}

				try {
					auto &buffer = *new (_alloc) Backend_gui::Pixel_buffer(_env, gui.pixel_buffers, { msg.view }, msg.size);

					msg.ptr = buffer.ram.local_addr<char>();

					return true;
				} catch (...) {
					Genode::error("insufficient resources to create buffer");
					return false;
				}
			}
			Genode::error("xxxx");
			return false;
		});
		return found;
	}
	case MessageConsole::TYPE_FREE_VIEW:
		if (msg.id == ID_VGA_VESA)
			return false;

		return apply_msg(msg.id, [&](auto &gui) {

			gui.free_buffer({ msg.view }, [&](auto &buffer) {
				Genode::destroy(_alloc, &buffer);
			});

			return true;
		});
	case MessageConsole::TYPE_RESOLUTION_CHANGE:
		/* not supported by now */
		if (msg.id == ID_VGA_VESA)
			return false;

		return apply_msg(msg.id, [&](auto &gui) {
			/* XXX adjust gui if _gui_non_vesa_ack is not very same as msg.width/msg.height ? */
			_gui_non_vesa_ack = Gui::Area(msg.width, msg.height);
			_gui_non_vesa_ack = _gui_non_vesa;
			return true;
		});
	case MessageConsole::TYPE_SWITCH_VIEW:
		/* XXX: For now, we only have one view. */
		return true;
	case MessageConsole::TYPE_GET_MODEINFO:
		return apply_msg(msg.id, [&](auto &gui) {
			return _vga_vesa.mode_info(msg, gui);
		});
	case MessageConsole::TYPE_PAUSE: /* all CPUs go idle */
	{
		_handle_fb(); /* refresh before going to sleep */

		Genode::Mutex::Guard guard(_mutex);
		_cpus_active = false;

		return true;
	}
	case MessageConsole::TYPE_RESUME: /* first of all sleeping CPUs woke up */
	{
		bool visible_vga_vesa = false;
		bool visible_others   = false;
		bool exists_others    = false;

		for_each_gui([&](auto &gui) {
			if (gui.id == ID_VGA_VESA && gui.visible)
				visible_vga_vesa = true;
			if (gui.id != ID_VGA_VESA)
				exists_others = true;
			if (gui.id != ID_VGA_VESA && gui.visible)
				visible_others = true;
		});

		if (visible_vga_vesa && _vga_vesa.idle() &&
		    exists_others && !visible_others) {
			for_each_gui([&](auto &gui) {
				if (gui.id == ID_VGA_VESA)
					return;

				/* unhide virtio gpu -  still required - */
				//Genode::log("unhide virtio gpu");
				gui.refresh(0, 0, 1, 1);
			});
		}

		if (visible_vga_vesa) {
			if (visible_others && _vga_vesa.idle()) {
				apply_msg(ID_VGA_VESA, [&](auto &gui) {
					Genode::log("hide vga_vesa window due to inactivity");
					gui.hide();
					return true;
				});
			} else {
				/* trigger "artificial" wakeup if vga/vesa idle */
				if (_vga_vesa.reactivate_key_pressed())
					_reactivate_periodic_timer();
			}
		}

		Genode::Mutex::Guard guard(_mutex);

		_cpus_active = true;

		return true;
	}
	case MessageConsole::TYPE_CONTENT_UPDATE:
	{
		bool const found = apply_msg(msg.id, [&](auto &gui) {
			if (msg.id == ID_VGA_VESA) {
				/* trigger "artificial" wakeup if vga/vesa idle */
				if (_vga_vesa.reactivate_key_pressed()) {
					/* trigger handle fb update with timeout programming */
					auto msg = MessageTimeout(_timer, 1000);
					receive(msg);
				}
			}

			if (msg.view == 0) {
				if (!msg.hide) {
					gui.refresh(msg.x, msg.y, msg.width, msg.height);
					return true;
				}

				if (msg.id != ID_VGA_VESA) {
					Genode::log("gui.hide() called");
					gui.hide();

					/* trigger "artificial" wakeup if vga/vesa idle */
					if (_vga_vesa.reactivate_key_pressed()) {
						/* trigger handle fb update with timeout programming */
						auto msg = MessageTimeout(_timer, 1000);
						receive(msg);
					}
				}
			}
			else if (msg.view == 1) {
				int hot_x = 0, hot_y = 0;

				if (!(_relative && !_absolute)) {
					hot_x = msg.hot_x;
					hot_y = msg.hot_y;
				}

				gui.mouse_shape(true /* XXX */,
				                hot_x, hot_y,
				                msg.width, msg.height);
			} else
				return false;
			return true;
		});

		if (!found)
			Genode::error("unknown graphical backend ", msg.id);

		return true;
	}
	default:
		return true;
	}
}


void Seoul::Console::_reactivate_periodic_timer()
{
	{
		Genode::Mutex::Guard guard(_mutex);

		if (!_cpus_active)
			_cpus_active = true;
	}

	MessageTimer msg(_timer, _mb.clock()->abstime(1, 1000));
	_mb.bus_timer.send(msg);
}


bool Seoul::Console::receive(MessageMemRegion &msg)
{
	bool const reactivate = _vga_vesa.reactivate_update(msg.page);

	if (reactivate) {
		//Genode::log("Reactivating periodic VGA/VESA update.");
		_reactivate_periodic_timer();
	}

	return false;
}


bool Seoul::Console::_sufficient_ram(Gui::Area const &cur, Gui::Area const &area)
{
	int64 const pixels  = area.count();

	if (!pixels)
		return true;

	auto const free_ram = _env.pd().avail_ram();
	auto const required = Genode::align_addr(uint64(pixels) * 4, 12);

	/* heuristics */
	if (required + 2 * 4096 > free_ram.value) {
		warning(cur, " -> ", area, " denied, requires ", required,
		        ", available ", free_ram);
		return false;
	}

	return true;
}


void Seoul::Console::_handle_gui_change()
{
	for_each_gui([&](auto &gui) {
		Gui::Rect const gui_win = gui.gui_window();

		if (gui.id == ID_VGA_VESA)
			return;

		if (gui_win.area == gui.fb_area)
			return;

		if (!_sufficient_ram(gui.fb_area, gui_win.area))
			return;

		if (!gui_win.area.valid()) {
			enum {
				BUTTON_POWER = 1U << 8,
				BUTTON_SLEEP = 1U << 9
			};

			log("trigger ACPI button power");

			MessageAcpiEvent event(MessageAcpiEvent::EventType::ACPI_EVENT_FIXED,
			                       BUTTON_POWER);

			_mb.bus_acpi_event.send(event);
		}

		/* send notification about new mode */
		MessageConsole msg(MessageConsole::TYPE_MODEINFO_UPDATE, gui.id);
		msg.x = msg.y = 0;
		msg.width  = gui_win.area.w;
		msg.height = gui_win.area.h;
		_mb.bus_console.send(msg);
	});
}


Genode::Milliseconds Seoul::Console::_handle_fb()
{
	Milliseconds reprogram_timer(0ULL);

	apply_msg(ID_VGA_VESA, [&](auto &gui) {
		reprogram_timer = _vga_vesa.handle_fb_gui(gui, _cpus_active);
		return true;
	});

	return reprogram_timer;
}


void Seoul::Console::_handle_input()
{
	bool valid_key = false;

	for_each_gui([&](auto &gui) {
		gui.gui.input.for_each_event([&] (Input::Event const &ev) {

			valid_key = true;

			if (mouse_event(ev)) {
				/* update PS2 mouse model */
				_input_to_ps2(ev);

				/* update virtio input model */
				_input_to_virtio(ev);
			}

			ev.handle_press([&] (Input::Keycode key, Genode::Codepoint) {
				if (key <= 0xee)
					_vkeyb.handle_keycode_press(key); });

			ev.handle_repeat([&] (Genode::Codepoint) {
				_vkeyb.handle_repeat(); });

			ev.handle_release([&] (Input::Keycode key) {
				if (key <= 0xee)
					_vkeyb.handle_keycode_release(key); });
		});
	});

	if (!valid_key)
		return;

	bool program_timeout = false;

	{
		Genode::Mutex::Guard guard(_mutex);

		program_timeout = !_cpus_active || _vga_vesa.reactivate_key_pressed();
		if (program_timeout)
			_cpus_active = true;
	}

	if (program_timeout) {
		MessageTimer msg(_timer, _mb.clock()->abstime(1, 1000));
		_mb.bus_timer.send(msg);
	}
}


bool Seoul::Console::receive(MessageTimeout &msg)
{
	if (msg.nr != _timer)
		return false;

	Milliseconds const timeout = _handle_fb();

	if (timeout.value) {
		MessageTimer msg_t(_timer, _mb.clock()->abstime(timeout.value, 1000));
		_mb.bus_timer.send(msg_t);
	}

	return true;
}


Seoul::Console::Console(Genode::Env &env, Genode::Allocator &alloc,
                        Motherboard &mb, Gui::Area const area,
                        Seoul::Guest_memory &guest_memory)
:
	_env(env),
	_mb(mb),
	_alloc(alloc),
	_memory(guest_memory),
	_gui_vesa(area), _gui_non_vesa(area), _gui_non_vesa_ack(area),
	_input_absolute(area),
	_vga_vesa(_memory, _binary_mono_tff_start)
{
	mb.bus_console  .add(this, receive_static<MessageConsole>);
	mb.bus_memregion.add(this, receive_static<MessageMemRegion>);
	mb.bus_timeout  .add(this, receive_static<MessageTimeout>);

	MessageTimer msg { };
	if (!mb.bus_timer.send(msg))
		Logging::panic("%s can't get a timer", __PRETTY_FUNCTION__);

	_timer = msg.nr;
}
