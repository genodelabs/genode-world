/**
 * \brief  SDL startup code
 * \author Josef Soentgen
 * \date   2017-08-17
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/heap.h>
#include <base/log.h>
#include <libc/component.h>


/* potentially needed by MESA (i965 DRM backend) */
Genode::Env                                      *genode_env;
static Genode::Constructible<Genode::Entrypoint>  signal_ep;


Genode::Entrypoint &genode_entrypoint()
{
	return *signal_ep;
}


/* provided by the application */
extern "C" int main(int argc, char ** argv, char **envp);


/* provided by our SDL backend */
extern void sdl_init_genode(Genode::Env &env);


void Libc::Component::construct(Libc::Env &env)
{
	sdl_init_genode(env);

	genode_env = &env;
	signal_ep.construct(*genode_env, 1024*sizeof(long), "sdl_signal_ep",
	                    Genode::Affinity::Location());
	Libc::with_libc([] () { main(0, nullptr, nullptr); });
}
