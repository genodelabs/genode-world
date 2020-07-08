/*
 * \brief  Frame-buffer driver for the OMAP4430 display-subsystem (HDMI)
 * \author Norman Feske
 * \date   2012-06-21
 */

/*
 * Copyright (C) 2012-2020 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/attached_rom_dataspace.h>
#include <base/attached_ram_dataspace.h>
#include <base/component.h>
#include <base/log.h>
#include <dataspace/client.h>
#include <timer_session/connection.h>
#include <capture_session/connection.h>

/* local includes */
#include <driver.h>

namespace Framebuffer {
	using namespace Genode;
	class Main;
};


struct Framebuffer::Main
{
	using Area  = Capture::Area;
	using Pixel = Capture::Pixel;

	Env &_env;

	Attached_rom_dataspace _config { _env, "config" };

	Framebuffer::Driver _driver { _env };

	Driver::Output _configured_output() const
	{
		Driver::Output value = Driver::OUTPUT_HDMI;

		if (_config.xml().attribute_value("output", String<8>()) == "LDC")
			value = Driver::OUTPUT_LCD;

		return value;
	}

	Area const _size { _config.xml().attribute_value("width", 1024u),
	                   _config.xml().attribute_value("height", 768u) };

	Attached_ram_dataspace _fb_ds { _env.ram(), _env.rm(),
	                                _driver.buffer_size(_size.w(), _size.h()),
	                                WRITE_COMBINED };

	addr_t const _fb_phys = Dataspace_client(_fb_ds.cap()).phys_addr();

	Capture::Connection _capture { _env };

	Capture::Connection::Screen _captured_screen { _capture, _env.rm(), _size };

	Timer::Connection _timer { _env };

	Signal_handler<Main> _timer_handler { _env.ep(), *this, &Main::_handle_timer };

	void _handle_timer()
	{
		Surface<Pixel> surface(_fb_ds.local_addr<Pixel>(), _size);

		_captured_screen.apply_to_surface(surface);
	}

	Main(Env &env) : _env(env)
	{
		if (!_driver.init(_size.w(), _size.h(), _configured_output(), _fb_phys)) {
			error("Could not initialize display");
			throw Exception();
		}

		_timer.sigh(_timer_handler);
		_timer.trigger_periodic(10*1000);
	}
};


void Component::construct(Genode::Env &env) { static Framebuffer::Main main(env); }
