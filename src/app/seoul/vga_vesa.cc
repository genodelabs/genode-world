/*
 * \brief  VGA/VESA adaptation to Genode interfaces
 * \author Alexander Boettcher
 * \author Markus Partheymueller
 * \author Norman Feske
 * \date   2022-01-09
 */

/*
 * Copyright (C) 2012 Intel Corporation
 * Copyright (C) 2013-2022 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 *
 * The code is partially based on the Seoul VMM, which is distributed
 * under the terms of the GNU General Public License version 2.
 *
 * Modifications by Intel Corporation are contributed under the terms and
 * conditions of the GNU General Public License version 2.
 */

#include "vga_vesa.h"


Genode::Milliseconds Seoul::Vga_vesa::_handle_vga_mode(Backend_gui &gui,
                                                       bool const cpus_active)
{
	Genode::Color cursor_color(255,255,255);
	bool          cursor_show = false;
	int           cursor_x    = 0;
	int           cursor_y    = 0;

	if (_fb_state.vga_off || !cpus_active) {
		_fb_state.idle ++;
		return Milliseconds(0ULL);
	} else
		_fb_state.idle = 0;

	/* calculate cursor position, get color during loop below */
	if (_regs->cursor_pos > _regs->offset) {
		int pos = int(_regs->cursor_pos - _regs->offset);
		cursor_x = pos % 80;
		cursor_y = pos / 80;

		cursor_show = cursor_y < 25;
	}

	if (_fb_state.cmp_even)
		_fb_state.checksum1 = 0;
	else
		_fb_state.checksum2 = 0;

	Genode::Surface<Pixel_rgb888> _surface(reinterpret_cast<Pixel_rgb888 *>(gui.pixels),
	                                       gui.fb_mode.area);

	for (int y = 0, prev = 0; y < 25; y++) {
		for (int x = 0; x < 80; x++) {

			Text_painter::Position const where(x*8, y*15);
			auto  const pos_char   = _guest_fb+(_regs->offset << 1)
			                       + y * 80 * 2 + x * 2;
			uint8 const character  = *(uint8 *) (pos_char);
			auto  const colorvalue = *(pos_char + 1);

			char buffer[2] = { *pos_char, 0 };

			{
				char bg = (colorvalue & 0xf0) >> 4;
				if (bg == 0x8) bg = 0x7;

				unsigned lum = ((bg & 0x8) >> 3)*127;
				Genode::Color color(((bg & 0x4) >> 2)*127+lum, /* R+luminosity */
				                    ((bg & 0x2) >> 1)*127+lum, /* G+luminosity */
				                    ((bg & 0x1) >> 0)*127+lum  /* B+luminosity */);

				Gui::Rect rect(Gui::Point(x * 8, y * 15),
				               Gui::Area(8, 15));
				Box_painter::paint(_surface, rect, color);
			}

			char fg = colorvalue & 0xf;
			if (fg == 0x8) fg = 0x7;
			unsigned lum = ((fg & 0x8) >> 3)*127;
			Genode::Color color(((fg & 0x4) >> 2)*127+lum, /* R+luminosity */
			                    ((fg & 0x2) >> 1)*127+lum, /* G+luminosity */
			                    ((fg & 0x1) >> 0)*127+lum  /* B+luminosity */);

			/* get cursor color */
			if (cursor_x == x && cursor_y == y)
				cursor_color = color;

			switch (character) {
			case 0xb3: { /* | */
				Gui::Rect rect(Gui::Point(x*8 + 2, y*15 + 0), Gui::Area(2, 15));
				Box_painter::paint(_surface, rect, color);
				break;
			}
			case 0xb4: { /* -| */
				Gui::Rect rect(Gui::Point(x*8 + 0, y*15 + 7), Gui::Area(3, 2));
				Box_painter::paint(_surface, rect, color);
				rect = Gui::Rect(Gui::Point(x*8 + 2, y*15 + 0), Gui::Area(2, 15));
				Box_painter::paint(_surface, rect, color);
				break;
			}
			case 0xbf: { /* -. */
				Gui::Rect rect(Gui::Point(x*8 + 0, y*15 + 7), Gui::Area(3, 2));
				Box_painter::paint(_surface, rect, color);
				rect = Gui::Rect(Gui::Point(x*8 + 2, y*15 + 8), Gui::Area(2, 8));
				Box_painter::paint(_surface, rect, color);
				break;
			}
			case 0xc3: { /* |- */
				Gui::Rect rect(Gui::Point(x*8 + 2, y*15 + 7), Gui::Area(6, 2));
				Box_painter::paint(_surface, rect, color);
				rect = Gui::Rect(Gui::Point(x*8 + 2, y*15 + 0), Gui::Area(2, 15));
				Box_painter::paint(_surface, rect, color);
				break;
			}
			case 0xc4: { /* - */
				Gui::Rect rect(Gui::Point(x*8 + 0, y*15 + 7), Gui::Area(8, 2));
				Box_painter::paint(_surface, rect, color);
				break;
			}
			case 0xc0: { /* '- */
				Gui::Rect rect(Gui::Point(x*8 + 2, y*15 + 7), Gui::Area(6, 2));
				Box_painter::paint(_surface, rect, color);
				rect = Gui::Rect(Gui::Point(x*8 + 2, y*15), Gui::Area(2, 8));
				Box_painter::paint(_surface, rect, color);
				break;
			}
			case 0xda: { /* .- */
				Gui::Rect rect(Gui::Point(x*8 + 2, y*15 + 7), Gui::Area(6, 2));
				Box_painter::paint(_surface, rect, color);
				rect = Gui::Rect(Gui::Point(x*8 + 2, y*15 + 8), Gui::Area(2, 8));
				Box_painter::paint(_surface, rect, color);
				break;
			}
			case 0xd9: { /* -' */
				Gui::Rect rect(Gui::Point(x*8 + 0, y*15 + 7), Gui::Area(3, 2));
				Box_painter::paint(_surface, rect, color);
				rect = Gui::Rect(Gui::Point(x*8 + 2, y*15), Gui::Area(2, 8));
				Box_painter::paint(_surface, rect, color);
				break;
			}
			default:
				Text_painter::paint(_surface, where, _default_font, color, buffer);
				break;
			}

			/* Checksum for comparing */
			if (_fb_state.cmp_even) _fb_state.checksum1 += prev + character;
			else _fb_state.checksum2 += prev + character;

			prev = (prev + character) & 0xff;
		}
	}

	if (cursor_show) {
		bool show = !_last_cursor.blink;
		if (_last_cursor.x != cursor_x || _last_cursor.y != cursor_y ||
		    show) {
			/* - */
			Gui::Rect rect(Gui::Point(cursor_x * 8 + 0,
			                          cursor_y * 15 + 7), Gui::Area(8, 2));
			Box_painter::paint(_surface, rect, cursor_color);

			show = true;
		}

		_last_cursor = { .x = cursor_x, .y = cursor_y, .blink = show };
	}

	_fb_state.cmp_even = !_fb_state.cmp_even;

	/* compare checksums to detect changed buffer */
	if (_fb_state.checksum1 != _fb_state.checksum2) {
		_fb_state.unchanged = 0;
		gui.refresh(0, 0, gui.fb_mode.area.w(), gui.fb_mode.area.h());
		return Milliseconds(100ULL);
	}

	if (++_fb_state.unchanged < 10) {
		if (cursor_show)
			gui.refresh(_last_cursor.x * 8  + 0,
			            _last_cursor.y * 15 + 7, 8, 2);
		return Milliseconds(_fb_state.unchanged * 30);
	}

	/* if we copy the same data 10 times, unmap the text buffer from guest */
	_memory.detach(PHYS_FRAME_VGA_COLOR << 12,
	               FRAME_COUNT_COLOR << 12);

	_fb_state.vga_off = true;
	_fb_state.unchanged = 0;

	gui.refresh(0, 0, gui.fb_mode.area.w(), gui.fb_mode.area.h());

	return Milliseconds(0ULL);
}


Genode::Milliseconds Seoul::Vga_vesa::_handle_vesa_mode(Backend_gui &gui,
                                                        bool const cpus_active)
{
	if (!_fb_state.vga_off) {
		_memory.detach(PHYS_FRAME_VGA_COLOR << 12,
		               FRAME_COUNT_COLOR << 12);

		_fb_state.vga_off = true;
	}

	if (!cpus_active) {
		_fb_state.idle++;
		_fb_state.unchanged = 0;
		_fb_state.vesa_off = true;
		return Milliseconds(0ULL);
	}

	if (_fb_state.cmp_even)
		_fb_state.checksum1 = 0;
	else
		_fb_state.checksum2 = 0;

	for (unsigned i = 0, prev = 0; i < gui.fb_size() / 4; i++) {
		auto const add = prev + ((unsigned *)gui.pixels)[i];
		if (_fb_state.cmp_even)
			_fb_state.checksum1 += add;
		else
			_fb_state.checksum2 += add;

		prev = add;
	}

	_fb_state.cmp_even = !_fb_state.cmp_even;

	if (_fb_state.checksum1 == _fb_state.checksum2) {
		_fb_state.idle++;
		_fb_state.unchanged ++;
		if (_fb_state.unchanged > 10'000) {
			_fb_state.vesa_off = true;
			_fb_state.unchanged = 0;

			log("disable vga_vesa update to due same content");
			return Milliseconds(0ULL);
		}

		return Milliseconds((_fb_state.unchanged >= 15) ? 1000ULL : _fb_state.unchanged * 10);
	}

	_fb_state.idle = 0;

	gui.refresh(0, 0, gui.fb_mode.area.w(), gui.fb_mode.area.h());

	_fb_state.unchanged++;

	return Milliseconds((_fb_state.unchanged > 4) ? 4 * 10 : _fb_state.unchanged * 10);
}
