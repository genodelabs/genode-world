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

	unsigned const period_us = config_period_ms() * 1000;

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

	enum { STAY_ACTIVE = 10 };
	int timer_active = 0;

	/* signaled by input */
	void handle_input()
	{
		using namespace Input;

		/* forward the queue */
		parent_input.for_each_event([&] (Event const &e) {
			/*
			 * laggy pointing is upleasant, so signal the client if
			 * there is anything other than button presses queued,
			 * otherwise notify the client when the timer fires
			 */
			queue.add(e, (e.type() != Event::PRESS) && (e.type() != Event::RELEASE));
		});

		if (timer_active < 1) /* wake up the timer */
			timer.trigger_periodic(period_us);

		timer_active = STAY_ACTIVE;
	}

	Signal_handler<Main> input_handler =
		{ env.ep(), *this, &Main::handle_input };

	/* signaled by timer */
	void handle_timer()
	{
		if (queue.empty()) {
			--timer_active;
			/* stop the timer if nothing is happening */
			if (timer_active < 1)
				timer.trigger_periodic(0);
		} else
			queue.submit_signal();
	}

	Signal_handler<Main> timer_handler =
		{ env.ep(), *this, &Main::handle_timer };

	/**
	 * Constructor
	 */
	Main(Genode::Env &env) : env(env)
	{
		queue.enabled(true);

		/* register input handler */
		parent_input.sigh(input_handler);

		/* register timeout handler */
		timer.sigh(timer_handler);

		/* announce service */
		env.parent().announce(env.ep().manage(input_root));
	}
};


/***************
 ** Component **
 ***************/

Genode::size_t Component::stack_size() { return 4*1024*sizeof(Genode::addr_t); }

void Component::construct(Genode::Env &env) { static Input_normalizer::Main inst(env); }
