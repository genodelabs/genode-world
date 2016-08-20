/*
 * \brief  Server that duplicates log streams
 * \author Emery Hemingway
 * \date   2016-07-28
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <log_session/connection.h>
#include <root/component.h>
#include <base/component.h>
#include <base/session_label.h>
#include <base/heap.h>
#include <base/log.h>
#include <base/snprintf.h>
#include <util/list.h>

namespace Log_tee {

	using namespace Genode;
	class Session_component;
	class Root_component;

}


class Log_tee::Session_component : public Rpc_object<Log_session>
{
	private:

		/* craft our own connection to get a label in */
		struct Log_connection : Connection<Log_session>, Log_session_client
		{
			Log_connection(Env &env, char const *args)
			:
				Connection<Log_session>(env, session(env.parent(), args)),
				Log_session_client(cap())
			{ }
		} _log;

		char _prefix[Session_label::capacity()+3];

	public:

		Session_component(Env &env, Session_label const &label, char const *args)
		: _log(env, args)
		{ snprintf(_prefix, sizeof(_prefix), "[%s] ", label.string()); }

		size_t write(Log_session::String const &msg) override
		{
			/* write to the dedicated client log session */
			size_t n = _log.write(msg);

			/* write to our own log session */
			log(Cstring(_prefix), msg.string());

			return n;
		}
};


class Log_tee::Root_component :
	public Genode::Root_component<Log_tee::Session_component>
{
	private:

		Env &_env;

	protected:

		Log_tee::Session_component *_create_session(char const *args) override
		{
			Session_label const label = label_from_args(args);

			try { return new (md_alloc()) Session_component(_env, label, args); }
			catch (Genode::Parent::Service_denied) { throw Root::Unavailable(); }
		}

	public:

		Root_component(Env &env, Allocator &alloc)
		:
			Genode::Root_component<Log_tee::Session_component>(env.ep(), alloc),
			_env(env)
		{ }
};


Genode::size_t Component::stack_size() { return 2*1024*sizeof(Genode::addr_t); }

void Component::construct(Genode::Env &env)
{
	/*
	 * Client sessions are not allocated on seperate dataspaces,
	 * they are allocated on a heap against the component's own
	 * RAM quota. The session RAM donation is passed verbatim to
	 * the backend session.
	 */

	static Genode::Heap heap(env.ram(), env.rm());
	static Log_tee::Root_component root(env, heap);

	env.parent().announce(env.ep().manage(root));
}
