/*
 * \brief  Server-side proxy component providing a rom_session over a network.
 * \author Johannes Schlatow
 * \date   2016-02-15
 *
 * Usage scenario:
 * __________    ______________                 ______________    __________
 * | server | -> | remote_rom | -> (network) -> | remote_rom | -> | client |
 * |        |    |   server   |                 |   client   |    |        |
 * ----------    --------------                 --------------    ----------
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <base/log.h>
#include <base/env.h>
#include <base/heap.h>

#include <base/component.h>
#include <base/attached_rom_dataspace.h>

#include <backend_base.h>
#include <util.h>

namespace Remote_rom {
	using Genode::size_t;
	using Genode::Attached_rom_dataspace;

	class Rom_forwarder;
	struct Main;

	static char modulename[255];
	static bool binary = false;

};

struct Remote_rom::Rom_forwarder : Rom_forwarder_base
{
		Attached_rom_dataspace &_rom;
		Backend_server_base    &_backend;

		unsigned                _current_hash    { 0 };
		bool                    _transmitting    { false };
		bool                    _update_received { false };

		Attached_rom_dataspace &_config;

		Rom_forwarder(Attached_rom_dataspace &rom, Backend_server_base &backend,
		              Attached_rom_dataspace &config)
			: _rom(rom), _backend(backend), _config(config)
		{
			_backend.register_forwarder(this);

			/* on startup, send an update message to remote client */
			update();
		}

		void start_transmission()  { _transmitting = true; }
		void finish_transmission()
		{
			_transmitting = false;

			/* if we received and update during transmission,
			 * we are now able to process it */
			if (_update_received)
				update();
		}

		const char *module_name() const { return modulename; }

		void update()
		{
			if (_transmitting) {
				/* if transmission is ongoing, do not update ROM */
				_update_received = true;
				return;
			}
			_update_received = false;

			/* refresh dataspace if valid */
			_rom.update();

			if (_rom.is_valid()) {
				_current_hash = cksum(_rom.local_addr<char>(), content_size());

				/* trigger backend_server */
				_backend.send_update();
			}
		}

		unsigned content_hash() const
		{
			return _current_hash;
		}

		size_t content_size() const
		{
			if (_rom.is_valid()) {
				if (binary)
					return _rom.size();
				else
					return Genode::min(Genode::strlen(_rom.local_addr<char>()),
					                   _rom.size());
			}
			return 0;
		}

		size_t transfer_content(char *dst, size_t dst_len, size_t offset=0) const
		{
			if (_rom.is_valid()) {
				size_t const len = Genode::min(dst_len, content_size()-offset);
				Genode::memcpy(dst, _rom.local_addr<char>() + offset, len);
				/* clear remaining buffer to prevent data leakage */
				if (dst_len > len) {
					Genode::memset(dst + len, 0, dst_len-len);
				}
				return dst_len;
			}
			return 0;
		}
};

struct Remote_rom::Main
{
	Genode::Env    &_env;
	Genode::Heap    _heap   = { &_env.ram(), &_env.rm() };
	Attached_rom_dataspace _config = { _env, "config" };

	Attached_rom_dataspace _rom;
	Rom_forwarder          _forwarder;

	Genode::Signal_handler<Rom_forwarder> _dispatcher = {_env.ep(), _forwarder,
	                                                     &Rom_forwarder::update};

	Main(Genode::Env &env)
		: _env(env),
	     _rom(env, modulename),
	     _forwarder(_rom, backend_init_server(env, _heap, _config.xml()),
	                _config)
	{
		/* register update dispatcher */
		_rom.sigh(_dispatcher);
	}
};

namespace Component {
	Genode::size_t stack_size()    { return 2*1024*sizeof(long); }


	void construct(Genode::Env &env)
	{
		using Remote_rom::modulename;

		Genode::Attached_rom_dataspace config = { env, "config" };
		try {
			Genode::Xml_node remote_rom = config.xml().sub_node("remote_rom");
			remote_rom.attribute("name").value(modulename, sizeof(modulename));
			try {
				remote_rom.attribute("binary").value(&Remote_rom::binary);
			} catch (...) { }
		} catch (...) {
			Genode::error("No ROM module configured!");
		}

		static Remote_rom::Main main(env);
	}
}
