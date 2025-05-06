/*
 * \brief  Nic bus service
 * \author Emery Hemingway
 * \date   2019-04-10
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* local includes */
#include "session_component.h"

/* Genode */
#include <root/component.h>
#include <base/attached_rom_dataspace.h>
#include <os/session_policy.h>
#include <base/component.h>

namespace Nic_bus {
	using namespace Net;
	using namespace Genode;
	class Root;
	struct Main;
}


class Nic_bus::Root : public Genode::Root_component<Nic_bus::Session_component>
{
	private:

		Genode::Env &_env;

		Attached_rom_dataspace _config_rom { _env, "config" };

		Session_bus _bus { };

	protected:

		Create_result _create_session(const char *args) override
		{
			using namespace Genode;

			_config_rom.update();

			Session_label  label  { label_from_args(args) };
			Session_policy policy { label, _config_rom.xml() };

			return *new (md_alloc())
				Session_component(_env.ep(), _env.ram(), _env.rm(),
				                  ram_quota_from_args(args),
				                  cap_quota_from_args(args),
				                  Tx_size{Arg_string::find_arg(args, "tx_buf_size").ulong_value(0)},
				                  Rx_size{Arg_string::find_arg(args, "rx_buf_size").ulong_value(0)},
				                  _bus,
				                  label);
		}

	public:

		Root(Genode::Env        &env,
		     Genode::Allocator  &md_alloc)
		: Genode::Root_component<Nic_bus::Session_component>(env.ep(), md_alloc),
		  _env(env)
		{ }
};


struct Nic_bus::Main
{
	Sliced_heap _sliced_heap;
	Nic_bus::Root _root;

	Main(Genode::Env &env)
	: _sliced_heap(env.ram(), env.rm()),
	  _root(env, _sliced_heap)
	{
		env.parent().announce(env.ep().manage(_root));
	}
};


void Component::construct(Genode::Env &env) { static Nic_bus::Main inst(env); }
