/*
 * \brief  Manager of all VM requested console functionality
 * \author Markus Partheymueller
 * \author Norman Feske
 * \author Alexander Boettcher
 * \date   2012-07-31
 */

/*
 * Copyright (C) 2011-2017 Genode Labs GmbH
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

static struct {
	bool active = false;
} cpu_state;


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
		static int ox = 0, oy = 0;
		rx = x - ox; ry = y - oy;
		ox = x; oy = y;
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
	if (_motherboard()->bus_input.send(msg) && !_relative)
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
			unsigned tx = _input_absolute.w() * x / _gui_non_vesa_ack.w();
			unsigned ty = _input_absolute.h() * y / _gui_non_vesa_ack.h();

			if (x < 0 || y < 0)
				return;

			if (tx > _input_absolute.w()) tx = _input_absolute.w();
			if (ty > _input_absolute.h()) ty = _input_absolute.h();

			MessageInput msg(0x10002, tx & mask, ty & mask);
			if (_motherboard()->bus_input.send(msg) && !_absolute) {
				_absolute = true;
				/* store absolute dimension of virtio input device */
				_input_absolute = Gui::Area(msg.data, msg.data2);
			}
		});
		ev.handle_wheel([&](int, int z) {
			MessageInput msg(0x10002, 1u << 28, z);
			_motherboard()->bus_input.send(msg);
		});

		return;
	}

	MessageInput msg(0x10002, data, data2);
	_motherboard()->bus_input.send(msg);
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

			if (msg.id != ID_VGA_VESA)
				gui.gui.mode_sigh(_signal_gui);
		}

		apply_msg(msg.id, [&](auto &gui) {
			/* vga_vesa re-creation is not supported */
			if (!create_new && msg.id != ID_VGA_VESA) {
				gui.resize(_env, gui.gui.mode());
				_gui_non_vesa = gui.gui.mode().area;
			}

			msg.ptr  = (char *)gui.pixels;
			msg.size = gui.fb_size();
			return true;
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
				Genode::Ram_dataspace_capability ds { };
				try {
					msg.ptr  = nullptr;

					ds = _env.ram().alloc(msg.size);
					msg.ptr = _env.rm().attach(ds);

					new (_alloc) Backend_gui::Pixel_buffer(gui.pixel_buffers, Backend_gui::Pixel_buffer::Id { msg.view }, ds);

					return true;
				} catch (...) {
					Genode::error("insufficient resources to create buffer");
					if (ds.valid())
						_env.ram().free(ds);
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

			Backend_gui::Pixel_buffer::Id const id { msg.view };
			gui.free_buffer(id, [&](Backend_gui::Pixel_buffer &buffer) {
				_env.ram().free(buffer.ds);
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
		_handle_fb(); /* refresh before going to sleep */
		cpu_state.active = false;
		return true;
	case MessageConsole::TYPE_RESUME: /* first of all sleeping CPUs woke up */
	{
		unsigned gui_count = 0;
		bool vga_vesa_visible = false;

		for_each_gui([&](auto &gui) {
			gui_count ++;
			if (gui.id == ID_VGA_VESA && gui.visible)
				vga_vesa_visible = true;
		});

		if (gui_count > 1 && vga_vesa_visible && _vga_vesa.idle()) {
			apply_msg(ID_VGA_VESA, [&](auto &gui) {
				Genode::log("hide vga_vesa window due to inactivity");
				gui.hide();
				return true;
			});
		}

		if (vga_vesa_visible)
			_reactivate_periodic_timer();

		return true;
	}
	case MessageConsole::TYPE_CONTENT_UPDATE:
	{
		bool const found = apply_msg(msg.id, [&](auto &gui) {
			if (msg.view == 0)
				gui.refresh(msg.x, msg.y, msg.width, msg.height);
			else if (msg.view == 1) {
				int x = 0, y = 0;

				/* makes mouse (somewhat) visible if solely relative input is used */
				if (_relative && !_absolute) {
					x = gui.last_host_pos.x() - msg.x;
					y = gui.last_host_pos.y() - msg.y;
				}

				gui.mouse_shape(true /* XXX */,
				                x, y,
				                msg.width, msg.height);
			} else
				return false;
			return true;
		});

		/* if the fb updating was off, reactivate timer - move into vga/vesa class XXX */
		if (msg.id == ID_VGA_VESA && !cpu_state.active)
			_reactivate_periodic_timer();

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
	cpu_state.active = true;

	MessageTimer msg(_timer, _unsynchronized_motherboard.clock()->abstime(1, 1000));
	_unsynchronized_motherboard.bus_timer.send(msg);
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


void Seoul::Console::_handle_gui_change()
{
	for_each_gui([&](auto &gui) {
		Framebuffer::Mode const gui_mode = gui.gui.mode();

		if (gui.id == ID_VGA_VESA)
			return;

		if (gui_mode.area == gui.fb_mode.area)
			return;

		int64 const pixdiff = gui_mode.area.count() - gui.fb_mode.area.count();

		if (pixdiff > 0 && (Genode::align_addr(uint64(pixdiff) * 4, 12) >
		                    _env.pd().avail_ram().value + 8192)) {
			warning(gui.fb_mode.area, " -> ", gui_mode.area, " denied,"
			        " requires ", Genode::align_addr(pixdiff * 4, 12),
			        ", availalble ", _env.pd().avail_ram().value, " + 8k");
			return;
		}

		/* send notification about new mode */
		MessageConsole msg(MessageConsole::TYPE_MODEINFO_UPDATE, gui.id);
		msg.x = msg.y = 0;
		msg.width  = gui_mode.area.w();
		msg.height = gui_mode.area.h();
		_motherboard()->bus_console.send(msg);
	});
}


Genode::Milliseconds Seoul::Console::_handle_fb()
{
	Milliseconds reprogram_timer(0ULL);

	apply_msg(ID_VGA_VESA, [&](auto &gui) {
		reprogram_timer = _vga_vesa.handle_fb_gui(gui, cpu_state.active);
		return true;
	});

	return reprogram_timer;
}


void Seoul::Console::_handle_input()
{
	for_each_gui([&](auto &gui) {
		gui.input.for_each_event([&] (Input::Event const &ev) {

			if (!cpu_state.active) {
				cpu_state.active = true;
				MessageTimer msg(_timer, _motherboard()->clock()->abstime(1, 1000));
				_motherboard()->bus_timer.send(msg);
			}

			if (mouse_event(ev)) {
				ev.handle_absolute_motion([&] (int x, int y) {
					gui.last_host_pos = Genode::Point<unsigned>(x, y);
				});

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
}


void Seoul::Console::register_host_operations(Motherboard &motherboard)
{
	motherboard.bus_console  .add(this, receive_static<MessageConsole>);
	motherboard.bus_memregion.add(this, receive_static<MessageMemRegion>);
	motherboard.bus_timeout  .add(this, receive_static<MessageTimeout>);

	MessageTimer msg;
	if (!motherboard.bus_timer.send(msg))
		Logging::panic("%s can't get a timer", __PRETTY_FUNCTION__);

	_timer = msg.nr;
}


bool Seoul::Console::receive(MessageTimeout &msg)
{
	if (msg.nr != _timer)
		return false;

	Milliseconds const timeout = _handle_fb();

	if (timeout.value) {
		MessageTimer msg_t(_timer, _unsynchronized_motherboard.clock()->abstime(timeout.value, 1000));
		_unsynchronized_motherboard.bus_timer.send(msg_t);
	}

	return true;
}


Seoul::Console::Console(Genode::Env &env, Genode::Allocator &alloc,
                        Synced_motherboard &mb,
                        Motherboard &unsynchronized_motherboard,
                        Gui::Area const area,
                        Seoul::Guest_memory &guest_memory)
:
	_env(env),
	_unsynchronized_motherboard(unsynchronized_motherboard),
	_motherboard(mb),
	_alloc(alloc),
	_memory(guest_memory),
	_gui_vesa(area), _gui_non_vesa(area), _gui_non_vesa_ack(area),
	_input_absolute(area),
	_vga_vesa(_memory, _binary_mono_tff_start)
{ }
