/*
 * \brief  VGA/VESA adaptation to Genode interfaces
 * \author Alexander Boettcher
 * \author Markus Partheymueller
 * \author Norman Feske
 * \date   2022-01-09
 */

/*
 * Copyright (C) 2012 Intel Corporation
 * Copyright (C) 2013-2024 Genode Labs GmbH
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
	static Genode::Color cursor_color { 255, 255, 255, 255 };
	static Genode::Color cursor_clear {   0,   0,   0,   0 };

	bool cursor_show = false;
	int  cursor_x    = 0;
	int  cursor_y    = 0;

	/* calculate cursor position, get color during loop below */
	if (_regs && _regs->cursor_pos > _regs->offset) {
		int pos = int(_regs->cursor_pos - _regs->offset);
		cursor_x = pos % 80;
		cursor_y = pos / 80;

		cursor_show = cursor_y < 25;

		if (cursor_color == cursor_clear) {
			cursor_color = { 255, 255, 255, 255 };
			cursor_clear = {   0,   0,   0,   0 };
		}
	}

	Genode::Surface<Pixel_rgb888> surface(reinterpret_cast<Pixel_rgb888 *>(gui.pixels),
	                                      gui.fb_area);

	if (_fb_state.vga_off || !cpus_active) {
		_fb_state.idle ++;

		if (cursor_show) {
			_print_cursor(cursor_color, cursor_clear, surface, cursor_x, cursor_y);

			gui.refresh(_last_cursor.x * 8  + 0,
			            _last_cursor.y * 15 + 7, 8, 2);

		}

		return Milliseconds(1000ULL);
	} else
		_fb_state.idle = 0;

	if (_fb_state.cmp_even)
		_fb_state.checksum1 = 0;
	else
		_fb_state.checksum2 = 0;

	auto color_from_palette_index = [&] (auto const idx) -> Color
	{
		uint8_t const lum = ((idx & 0x8) >> 3) * 127;
		return {
			.r = uint8_t(((idx & 0x4) >> 2) * 127 + lum), /* R+luminosity */
			.g = uint8_t(((idx & 0x2) >> 1) * 127 + lum), /* G+luminosity */
			.b = uint8_t(((idx & 0x1) >> 0) * 127 + lum), /* B+luminosity */
			.a = 255
		};
	};

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

				Gui::Rect rect(Gui::Point(x * 8, y * 15),
				               Gui::Area(8, 15));

				auto color = color_from_palette_index(bg);
				Box_painter::paint(surface, rect, color);

				/* get cursor color */
				if (cursor_x == x && cursor_y == y)
					cursor_clear = color;
			}

			char fg = colorvalue & 0xf;
			if (fg == 0x8) fg = 0x7;

			Color const color = color_from_palette_index(fg);

			/* get cursor color */
			if (cursor_x == x && cursor_y == y)
				cursor_color = color;

			switch (character) {
			case 0xb3: { /* | */
				Gui::Rect rect(Gui::Point(x*8 + 2, y*15 + 0), Gui::Area(2, 15));
				Box_painter::paint(surface, rect, color);
				break;
			}
			case 0xb4: { /* -| */
				Gui::Rect rect(Gui::Point(x*8 + 0, y*15 + 7), Gui::Area(3, 2));
				Box_painter::paint(surface, rect, color);
				rect = Gui::Rect(Gui::Point(x*8 + 2, y*15 + 0), Gui::Area(2, 15));
				Box_painter::paint(surface, rect, color);
				break;
			}
			case 0xbf: { /* -. */
				Gui::Rect rect(Gui::Point(x*8 + 0, y*15 + 7), Gui::Area(3, 2));
				Box_painter::paint(surface, rect, color);
				rect = Gui::Rect(Gui::Point(x*8 + 2, y*15 + 8), Gui::Area(2, 8));
				Box_painter::paint(surface, rect, color);
				break;
			}
			case 0xc3: { /* |- */
				Gui::Rect rect(Gui::Point(x*8 + 2, y*15 + 7), Gui::Area(6, 2));
				Box_painter::paint(surface, rect, color);
				rect = Gui::Rect(Gui::Point(x*8 + 2, y*15 + 0), Gui::Area(2, 15));
				Box_painter::paint(surface, rect, color);
				break;
			}
			case 0xc4: { /* - */
				Gui::Rect rect(Gui::Point(x*8 + 0, y*15 + 7), Gui::Area(8, 2));
				Box_painter::paint(surface, rect, color);
				break;
			}
			case 0xc0: { /* '- */
				Gui::Rect rect(Gui::Point(x*8 + 2, y*15 + 7), Gui::Area(6, 2));
				Box_painter::paint(surface, rect, color);
				rect = Gui::Rect(Gui::Point(x*8 + 2, y*15), Gui::Area(2, 8));
				Box_painter::paint(surface, rect, color);
				break;
			}
			case 0xda: { /* .- */
				Gui::Rect rect(Gui::Point(x*8 + 2, y*15 + 7), Gui::Area(6, 2));
				Box_painter::paint(surface, rect, color);
				rect = Gui::Rect(Gui::Point(x*8 + 2, y*15 + 8), Gui::Area(2, 8));
				Box_painter::paint(surface, rect, color);
				break;
			}
			case 0xd9: { /* -' */
				Gui::Rect rect(Gui::Point(x*8 + 0, y*15 + 7), Gui::Area(3, 2));
				Box_painter::paint(surface, rect, color);
				rect = Gui::Rect(Gui::Point(x*8 + 2, y*15), Gui::Area(2, 8));
				Box_painter::paint(surface, rect, color);
				break;
			}
			default:
				Text_painter::paint(surface, where, _default_font, color, buffer);
				break;
			}

			/* Checksum for comparing */
			if (_fb_state.cmp_even) _fb_state.checksum1 += prev + character;
			else _fb_state.checksum2 += prev + character;

			prev = (prev + character) & 0xff;
		}
	}

	if (cursor_show)
		_print_cursor(cursor_color, cursor_clear, surface, cursor_x, cursor_y);

	_fb_state.cmp_even = !_fb_state.cmp_even;

	/* compare checksums to detect changed buffer */
	if (_fb_state.checksum1 != _fb_state.checksum2) {
		_fb_state.unchanged = 0;
		gui.refresh(0, 0, gui.fb_area.w, gui.fb_area.h);
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

	gui.refresh(0, 0, gui.fb_area.w, gui.fb_area.h);

	return Milliseconds(1000ULL);
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

			log("disable vga_vesa update due to same content");
			return Milliseconds(0ULL);
		}

		return Milliseconds((_fb_state.unchanged >= 15) ? 1000ULL : _fb_state.unchanged * 10);
	}

	_fb_state.idle = 0;

	gui.refresh(0, 0, gui.fb_area.w, gui.fb_area.h);

	_fb_state.unchanged++;

	return Milliseconds((_fb_state.unchanged > 4) ? 4 * 10 : _fb_state.unchanged * 10);
}
