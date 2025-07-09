/*
 * \brief  Component providing a Terminal session via SSH
 * \author Josef Soentgen
 * \author Pirmin Duss
 * \author Tomasz Gajewski
 * \date   2019-05-29
 */

/*
 * Copyright (C) 2018 Genode Labs GmbH
 * Copyright (C) 2019-2021 gapfruit AG
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SSH_TERMINAL_ROOT_COMPONENT_H_
#define _SSH_TERMINAL_ROOT_COMPONENT_H_

/* Genode includes */
#include <root/component.h>
#include <os/session_policy.h>

/* local includes */
#include "session_component.h"
#include "server.h"


namespace Terminal
{
	using namespace Genode;

	class Root_component;
}


class Terminal::Root_component : public Genode::Root_component<Session_component>
{
	private:

		Genode::Env &_env;

		Genode::Attached_rom_dataspace _config_rom   { _env, "config" };
		Genode::Node                   _config       { _config_rom.node() };

		Genode::Heap        _logins_alloc { _env.ram(), _env.rm() };
		Ssh::Login_registry _logins       { _logins_alloc };

		Ssh::Server _server { _env, _config, _logins };

		Genode::Signal_handler<Terminal::Root_component>  _config_sigh  {
			_env.ep(), *this, &Terminal::Root_component::_handle_config_update };

		void _handle_config_update()
		{
			_config_rom.update();
			if (!_config_rom.valid()) { return; }

			Libc::with_libc([&] () {
				{
					Util::Pthread_mutex::Guard guard(_logins.mutex());
					_logins.import(_config_rom.node());
				}

				_server.update_config(_config_rom.node());
			});
		}

	protected:

		Create_result _create_session(char const * const args)
		{
			return with_matching_policy(label_from_args(args), _config,
				[&] (Node const &policy) -> Create_result {
					Ssh::Terminal_name const term_name =
						policy.attribute_value("terminal_name", Ssh::Terminal_name());

					if (!term_name.valid())
						return Create_error::DENIED;

					auto s = new (md_alloc()) Session_component(_env, 4096, term_name);

					try {
						Libc::with_libc([&] () { _server.attach_terminal(*s); });
						return *s;
					}
					catch (...) {
						_destroy_session(*s);
						return Create_error::DENIED;
					}
				},
				[&] () -> Create_result { return Create_error::DENIED; });
		}

		void _destroy_session(Session_component &s)
		{
			Libc::with_libc([&] () { _server.detach_terminal(s); });
			Genode::destroy(md_alloc(), &s);
		}

	public:

		Root_component(Genode::Env       &env,
		               Genode::Allocator &md_alloc)
		:
			Genode::Root_component<Session_component>(&env.ep().rpc_ep(),
			                                          &md_alloc),
			_env(env)
		{
			_config_rom.sigh(_config_sigh);
			_handle_config_update();
		}
};

#endif  /* _SSH_TERMINAL_ROOT_COMPONENT_H_ */
