/*
 * \brief  Entropy visualizer
 * \author Emery Hemingway
 * \date   2018-11-09
 */

/*
 * Copyright (C) 2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <terminal_session/connection.h>
#include <nitpicker_session/connection.h>
#include <framebuffer_session/connection.h>
#include <base/attached_dataspace.h>
#include <base/heap.h>
#include <base/log.h>
#include <base/component.h>


namespace Nit_entropy {
	using namespace Genode;
	typedef Surface_base::Point Point;
	typedef Surface_base::Area  Area;
	typedef Surface_base::Rect  Rect;

	struct Main;
};


struct Nit_entropy::Main
{
	enum { WIDTH = 256, HEIGHT = 256 };

	Genode::Env &_env;

	Heap _heap { _env.ram(), _env.rm() };

	Nitpicker::Connection _nitpicker { _env };

	Framebuffer::Session &_fb = *_nitpicker.framebuffer();

	Terminal::Connection _entropy { _env, "entropy" };

	Dataspace_capability _fb_ds_cap()
	{
		using Framebuffer::Mode;
		Mode mode(WIDTH, HEIGHT, Mode::RGB565);
		_nitpicker.buffer(mode, false);
		return _fb.dataspace();
	}

	Attached_dataspace _fb_ds { _env.rm(), _fb_ds_cap() };

	Nitpicker::Session::View_handle _view = _nitpicker.create_view();

	void _refresh() { _fb.refresh(0, 0, WIDTH, HEIGHT); }

	void _plot()
	{
		uint16_t *pixels = _fb_ds.local_addr<uint16_t>();
		static uint8_t buf[HEIGHT];
		size_t n = _entropy.read(buf, sizeof(buf));
		if (n != sizeof(buf)) {
			Genode::error("read ", n, " bytes of entropy");
		}

		for (int i = 0; i < HEIGHT; ++i) {
			uint16_t *row = &pixels[i*WIDTH];
			row[buf[i]] = ~row[buf[i]];
		}

		_refresh();
	}

	Signal_handler<Main> _sync_handler {
		_env.ep(), *this, &Main::_plot };

	Main(Env &env) : _env(env)
	{
		_nitpicker.enqueue<Nitpicker::Session::Command::Geometry>(
			_view, Rect(Point(0, 0), Area (WIDTH, HEIGHT)));

		_nitpicker.enqueue<Nitpicker::Session::Command::To_front>(
			_view, Nitpicker::Session::View_handle());
		_nitpicker.execute();

		_fb.sync_sigh(_sync_handler);
	}
};


void Component::construct(Genode::Env &env) {
	static Nit_entropy::Main inst(env); }
