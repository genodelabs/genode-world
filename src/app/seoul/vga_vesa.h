/*
 * \brief  VGA/VESA adaptation to Genode interfaces
 * \author Alexander Boettcher
 * \author Markus Partheymueller
 * \author Norman Feske
 * \date   2022-01-09
 */

/*
 * Copyright (C) 2013-2022 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 *
 * The code is partially based on the Seoul VMM, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _VGA_VESA_H_
#define _VGA_VESA_H_

#include <base/duration.h>
#include <os/pixel_rgb888.h>

/* nitpicker graphics backend */
#include <nitpicker_gfx/tff_font.h>
#include <nitpicker_gfx/box_painter.h>

#include <nul/message.h> /* VgaRegs */

#include "gui.h"
#include "guest_memory.h"

namespace Seoul {
	class Vga_vesa;
	using Genode::Pixel_rgb888;
	using Genode::Milliseconds;
}

class Seoul::Vga_vesa
{
	private:

		/*
		 * Noncopyable
		 */
		Vga_vesa(Vga_vesa const &);
		Vga_vesa &operator = (Vga_vesa const &);

		Seoul::Guest_memory &_memory;
		VgaRegs             *_regs     { nullptr };
		char                *_guest_fb { nullptr };
		uint64               _fb_phys_base { 0 };

		enum {
			PHYS_FRAME_VGA       = 0xa0,
			PHYS_FRAME_VGA_COLOR = 0xb8,
			FRAME_COUNT_COLOR    = 0x8
		};

		Tff_font::Static_glyph_buffer<4096> _glyph_buffer { };
		Tff_font                            _default_font;

		struct {
			uint64   checksum1;
			uint64   checksum2;
			unsigned unchanged;
			unsigned idle;
			bool     cmp_even;
			bool     vga_off;
		} _fb_state {
			.checksum1 = 0,
			.checksum2 = 0,
			.unchanged = 0,
			.idle      = 0,
			.cmp_even  = true,
			.vga_off   = false
		};

		struct {
			int x; int y; bool blink;
		} _last_cursor { };

		Milliseconds _handle_vga_mode (Backend_gui &, bool);
		Milliseconds _handle_vesa_mode(Backend_gui &, bool);

	public:

		Vga_vesa(Guest_memory &memory, char * binary_mono_ttf_start)
		:
			_memory(memory),
			_default_font(binary_mono_ttf_start, _glyph_buffer)
		{ }

		void init(VgaRegs *regs, char *guest_fb, uint64 fb_phys_base)
		{
			_regs = regs;
			_guest_fb = guest_fb;
			_fb_phys_base = fb_phys_base;
		}

		Milliseconds handle_fb_gui(Backend_gui &gui, bool const cpus_active)
		{
			bool const skip_update = _fb_state.vga_off;

			if (!_guest_fb || !_regs)
				return Milliseconds(0ULL);

			if (_regs->mode == 0 /* TEXT_MODE */)
				return _handle_vga_mode(gui, cpus_active);
			else
				return _handle_vesa_mode(gui, cpus_active);
		}

		bool reactivate_update(uint64 const access)
		{
			/* we had a fault in the text framebuffer */
			bool reactivate = (access >= PHYS_FRAME_VGA &&
			                   access < PHYS_FRAME_VGA_COLOR + FRAME_COUNT_COLOR);

			if (reactivate)
				_fb_state.vga_off = false;

			return reactivate;
		}

		bool mode_info(MessageConsole &msg, Backend_gui &gui)
		{
			enum {
				MEMORY_MODEL_TEXT = 0,
				MEMORY_MODEL_DIRECT_COLOR = 6,
			};

			if (!msg.info)
				return false;

			/*
			 * We supply two modes to the guest, text mode and one
			 * configured graphics mode 16-bit.
			 */
			if (msg.index == 0) {
				Genode::memset(msg.info, 0, sizeof(*msg.info));
				msg.info->_vesa_mode = 3;
				msg.info->attr = 0x1;
				msg.info->resolution[0] = 80;
				msg.info->resolution[1] = 25;
				msg.info->bytes_per_scanline = 80*2;
				msg.info->bytes_scanline = 80*2;
				msg.info->bpp = 4;
				msg.info->memory_model = MEMORY_MODEL_TEXT;
				msg.info->phys_base = PHYS_FRAME_VGA_COLOR << 12;
				msg.info->_phys_size = FRAME_COUNT_COLOR << 12;
				return true;

			} else if (msg.index == 1) {

				Genode::memset(msg.info, 0, sizeof(*msg.info));
				/*
				 * It's important to set the _vesa_mode field, otherwise the
				 * device model is going to ignore this mode.
				 */
				unsigned const bytes = gui.fb_mode.bytes_per_pixel();
				msg.info->_vesa_mode = 0x138;
				msg.info->attr = 0x39f;
				msg.info->resolution[0]      = gui.fb_mode.area.w();
				msg.info->resolution[1]      = gui.fb_mode.area.h();
				msg.info->bytes_per_scanline = gui.fb_mode.area.w()*bytes;
				msg.info->bytes_scanline     = gui.fb_mode.area.w()*bytes;
				msg.info->bpp = bytes * 8;
				msg.info->memory_model = MEMORY_MODEL_DIRECT_COLOR;
				msg.info->vbe1[0] =  0x8; /* red mask size */
				msg.info->vbe1[1] = 0x10; /* red field position */
				msg.info->vbe1[2] =  0x8; /* green mask size */
				msg.info->vbe1[3] =  0x8; /* green field position */
				msg.info->vbe1[4] =  0x8; /* blue mask size */
				msg.info->vbe1[5] =  0x0; /* blue field position */
				msg.info->vbe1[6] =  0x0; /* reserved mask size */
				msg.info->vbe1[7] =  0x0; /* reserved field position */
				msg.info->colormode = 0x0; /* direct color mode info */
				msg.info->phys_base = _fb_phys_base;
				msg.info->_phys_size = gui.fb_mode.area.count()*bytes;
				return true;
			}
			return false;
		}

		bool idle()
		{
			return _fb_state.idle > 500;
		}
};

#endif /* _VGA_VESA_H_ */
