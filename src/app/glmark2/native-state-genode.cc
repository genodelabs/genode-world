#include <libc/component.h>

#include <libc/args.h>
#include <stdlib.h>

#include <native-state-genode.h>

Genode::Env *genode_env;

extern char **environ;
extern "C" int main(int argc, char **argv, char **envp);

NativeStateGenode &NativeStateGenode::native_state(Genode::Env *env)
{
	static Genode::Constructible<NativeStateGenode> _native;

	if (_native.constructed())
		return *_native;

	if (env == nullptr && !_native.constructed()) {
		Genode::error("NativeStateGenode is not constructed");
		throw -1;
	}

	_native.construct(*env);

	return *_native;
}


static void component_construct(Libc::Env &env)
{
	int argc    = 0;
	char **argv = nullptr;
	char **envp = nullptr;

	populate_args_and_env(env, argc, argv, envp);

	environ = envp;

	/* construct */
	NativeStateGenode::native_state(&env);

	exit(main(argc, argv, envp));
}


void Libc::Component::construct(Libc::Env &env)
{
	genode_env = &env;

	Libc::with_libc([&] () { component_construct(env); });
}

