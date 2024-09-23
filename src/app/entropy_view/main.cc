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
#include <gui_session/connection.h>
#include <framebuffer_session/connection.h>
#include <base/attached_dataspace.h>
#include <base/heap.h>
#include <base/log.h>
#include <base/component.h>


namespace Entropy_view {
	using namespace Genode;
	typedef Surface_base::Point Point;
	typedef Surface_base::Area  Area;
	typedef Surface_base::Rect  Rect;

	struct Main;
};


struct Entropy_view::Main
{
	static constexpr Area _area { .w = 256, .h = 256 };

	Genode::Env &_env;

	Heap _heap { _env.ram(), _env.rm() };

	Gui::Connection _gui { _env };

	Terminal::Connection _entropy { _env, "entropy" };

	Dataspace_capability _fb_ds_cap()
	{
		_gui.buffer({ .area = _area, .alpha = false });
		return _gui.framebuffer.dataspace();
	}

	Attached_dataspace _fb_ds { _env.rm(), _fb_ds_cap() };

	Gui::Top_level_view _view { _gui, { { 0, 0 }, _area } };

	void _refresh() { _gui.framebuffer.refresh({ { 0, 0 }, _area }); }

	void _plot()
	{
		uint32_t *pixels = _fb_ds.local_addr<uint32_t>();
		static uint8_t buf[_area.h];
		size_t n = _entropy.read(buf, sizeof(buf));
		if (n != sizeof(buf)) {
			Genode::error("read ", n, " bytes of entropy");
		}

		for (unsigned i = 0; i < _area.h; ++i) {
			uint32_t *row = &pixels[i*_area.w];
			row[buf[i]] = ~row[buf[i]];
		}

		_refresh();
	}

	Signal_handler<Main> _sync_handler {
		_env.ep(), *this, &Main::_plot };

	Main(Env &env) : _env(env)
	{
		_gui.framebuffer.sync_sigh(_sync_handler);
		_sync_handler.local_submit();
	}
};


void Component::construct(Genode::Env &env) {
	static Entropy_view::Main inst(env); }
