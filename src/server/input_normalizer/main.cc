/*
 * \brief  Input event timing normalizer
 * \author Emery Hemingway
 * \author Christian Prochaska
 * \date   2016-03-18
 */

/*
 * Copyright (C) 2014-2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <timer_session/connection.h>
#include <input/component.h>
#include <input/event_queue.h>
#include <input_session/connection.h>
#include <os/static_root.h>
#include <base/component.h>
#include <base/attached_rom_dataspace.h>
#include <base/attached_dataspace.h>

namespace Input_normalizer {
	using namespace Genode;
	struct Main;
}

/******************
 ** Main program **
 ******************/

struct Input_normalizer::Main
{
	Genode::Env &env;

	unsigned config_period_ms()
	{
		enum { DEFAULT_PERIOD_MS = 200 };
		unsigned value = DEFAULT_PERIOD_MS;

		try {
			Attached_rom_dataspace config { env, "config" };
			config.xml().attribute("period_ms").value(&value);
		} catch (...) { }
		return value;
	}

	Microseconds const period_us =
		Microseconds{config_period_ms() * 1000};

	/* input session provided by our parent  */
	Input::Connection parent_input { env };

	/* input session provided to our client  */
	Input::Session_component client_input { env, env.ram() };

	Input::Event_queue &queue = client_input.event_queue();

	/*  attach root interface to the entry point */
	Static_root<Input::Session> input_root
		{ env.ep().manage(client_input) };

	/* Timer session for delaying events */
	Timer::Connection timer { env };

	/* submit after delay */
	void handle_timeout(Duration)
	{
		if (!queue.empty())
			queue.submit_signal();
	}

	Timer::One_shot_timeout<Main> burst_timeout =
		{ timer, *this, &Main::handle_timeout };

	/* signaled by input signal */
	void handle_input()
	{
		using namespace Input;

		/*
		 * laggy pointing is upleasant, so signal downstream if
		 * there is anything other than button presses upstream,
		 * otherwise notify downstream after a timeout
		 */
		bool notify = false;

		parent_input.for_each_event([&] (Event const &e) {
			if (!e.press() && !e.release())
				notify = true;
			enum { SUBMIT_NOW = false };
			queue.add(e, SUBMIT_NOW);
		});

		if (notify) {
			queue.submit_signal();
			burst_timeout.discard();
		} else if (!burst_timeout.scheduled()) {
			burst_timeout.schedule(period_us);
		}
	}

	Signal_handler<Main> input_handler =
		{ env.ep(), *this, &Main::handle_input };

	/**
	 * Constructor
	 */
	Main(Genode::Env &env) : env(env)
	{
		queue.enabled(true);

		/* register input handler */
		parent_input.sigh(input_handler);

		/* announce service */
		env.parent().announce(env.ep().manage(input_root));
	}
};


/***************
 ** Component **
 ***************/

Genode::size_t Component::stack_size() { return 4*1024*sizeof(Genode::addr_t); }

void Component::construct(Genode::Env &env) { static Input_normalizer::Main inst(env); }
