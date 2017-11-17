/*
 * \brief  Libretro frontend
 * \author Emery Hemingway
 * \date   2016-07-03
 */

/*
 * Copyright (C) 2016-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <libc/component.h>

/* Local includes */
#include "config.h"
#include "environment.h"
#include "dispatcher.h"

namespace Retro_frontend {

	static Dispatcher *dispatcher;

	void toggle_pause() {
		dispatcher->toggle_pause(); }

	void shutdown()
	{
		dispatcher->deinit();
		genv->parent().exit(0);
	}
}


/* each core will drive the stack differently, so be generous */
Genode::size_t Component::stack_size() { return 64*1024*sizeof(Genode::addr_t); }

void Libc::Component::construct(Libc::Env &env)
{
	using namespace Retro_frontend;

	genv = &env;

	config_rom.construct(env, "config");

	static Dispatcher inst;
	dispatcher = &inst;

	/* load and initialize core, configure signal handlers */
	dispatcher->handle_config();
}
