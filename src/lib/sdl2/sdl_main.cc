/*
 * \brief  Entry point for SDL applications with a main() function
 * \author Josef Soentgen
 * \date   2017-11-21
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/sleep.h>
#include <libc/component.h>
#include <libc/args.h>      /* populate_args_and_env() */

/* libc includes */
#include <stdlib.h> /* 'malloc' and 'exit' */

/* initial environment for the FreeBSD libc implementation */
extern char **environ;

/* provided by the application */
extern "C" int main(int argc, char *argv[], char *envp[]);


/* provided by our SDL backend */
extern void sdl_init_genode(Genode::Env &env);


Genode::Env *genode_env;


Genode::size_t Libc::Component::stack_size() { return 768 * 1024; }


void Libc::Component::construct(Libc::Env &env)
{
	int argc = 0;
	char **argv;
	char **envp;

	populate_args_and_env(env, argc, argv, envp);

	environ = envp;

	/* pass env to SDL backend */
	sdl_init_genode(env);

	Libc::with_libc([&] () {
		exit(main(argc, argv, envp));
	});
}
