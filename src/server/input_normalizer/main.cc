/*
 * \brief  Input event timing normalizer
 * \author Emery Hemingway
 * \author Christian Prochaska
 * \date   2016-03-18
 */

/*
 * Copyright (C) 2014-2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <timer_session/connection.h>
#include <input/component.h>
#include <input_session/connection.h>
#include <os/attached_dataspace.h>
#include <os/config.h>
#include <os/server.h>
#include <os/static_root.h>

namespace Input_normalizer {
	using namespace Genode;
	struct Main;
}

/******************
 ** Main program **
 ******************/

struct Input_normalizer::Main
{
	Server::Entrypoint &ep;

	/*
	 * Input session provided by our parent
	 */
	Input::Session_client parent_input
		{ env()->parent()->session<Input::Session>("ram_quota=16K") };
	Attached_dataspace input_dataspace { parent_input.dataspace() };

	/*
	 * Input session provided to our client
	 */
	Input::Session_component input_session_component;

	/*
	 * Attach root interface to the entry point
	 */
	Static_root<Input::Session> input_root { ep.manage(input_session_component) };

	/*
	 * Timer session for event timing
	 */
	Timer::Connection timer;

	void flush_input(unsigned)
	{
		Input::Event const * const events =
			input_dataspace.local_addr<Input::Event>();

		unsigned const num = parent_input.flush();
		for (unsigned i = 0; i < num; i++)
			input_session_component.submit(events[i]);
	}

	Signal_rpc_member<Main> timer_dispatcher =
		{ ep, *this, &Main::flush_input };

	/**
	 * Constructor
	 */
	Main(Server::Entrypoint &ep) : ep(ep)
	{
		input_session_component.event_queue().enabled(true);

		/*
		 * Get period from config
		 */
		enum { DEFAULT_PERIOD_MS = 200 };
		unsigned period_ms = DEFAULT_PERIOD_MS;
		period_ms = config()->xml_node().attribute_value("period_ms", period_ms);

		/*
		 * Register timeout handler
		 */
		timer.sigh(timer_dispatcher);
		timer.trigger_periodic(period_ms*1000); /* convert to microseconds */

		/*
		 * Announce service
		 */
		Genode::env()->parent()->announce(ep.manage(input_root));
	}

	~Main()
	{
		env()->parent()->close(parent_input);
	}

};


/************
 ** Server **
 ************/

namespace Server {

	char const *name() { return "input_normalizer_ep"; }

	size_t stack_size() { return 4*1024*sizeof(addr_t); }

	void construct(Entrypoint &ep) { static Input_normalizer::Main inst(ep); }
}
