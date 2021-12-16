/*
 * \brief  Startup code for golang runtime
 * \author Alexander Tormasov
 * \date   2021-12-16
 */

#include <libc/component.h>
#include <libc/args.h>

using namespace Genode;

extern char **genode_argv;
extern int genode_argc;
extern char **genode_envp;

/* initial environment for the FreeBSD libc implementation */
extern char **environ;

/* provided by the application */
extern "C" int main(int argc, char **argv, char **envp);

static void construct_component(Libc::Env &env)
{
	populate_args_and_env(env, genode_argc, genode_argv, genode_envp);

	environ = genode_envp;

	exit(main(genode_argc, genode_argv, genode_envp));
}


namespace Libc {

	void anon_init_file_operations(Genode::Env &env,
								   Xml_node const &config_accessor);
}

extern "C" void wait_for_continue();

void Libc::Component::construct(Libc::Env &env)
{
	// wait_for_continue();

	Libc::anon_init_file_operations(env, env.libc_config());

	Libc::with_libc([&]() { construct_component(env); });
}
