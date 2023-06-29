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
#include <pthread.h>

static int    argc;
static char **argv;
static char **envp;

/* initial environment for the FreeBSD libc implementation */
extern char **environ;

/* provided by the application */
extern "C" int main(int argc, char *argv[], char *envp[]);


/* provided by our SDL backend */
extern void sdl_init_genode(Genode::Env &env);



static void* sdl_main(void *)
{
	exit(main(argc, argv, envp));
	return nullptr;
}


void Libc::Component::construct(Libc::Env &env)
{
	populate_args_and_env(env, argc, argv, envp);

	environ = envp;

	/* pass env to SDL backend */
	sdl_init_genode(env);

	pthread_attr_t attr;
	pthread_t      main_thread;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 768 * 1024);

	if (pthread_create(&main_thread, &attr, sdl_main, nullptr)) {
		Genode::error("failed to create SDL main thread");
		exit(1);
	}
}
