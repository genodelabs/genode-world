#include <base/attached_rom_dataspace.h>
#include <libc/component.h>
#include <base/heap.h>
#include <base/log.h>
#include <base/thread.h>
#include <pthread.h>
#include "greeter_server.h"

enum { STACK_SIZE = 0xF000 };

namespace Grpc_server {
	using namespace Genode;

	class Server_main;
}

static void *start_func(void* envp)
{
	Genode::Env& env = *(reinterpret_cast<Genode::Env*>(envp));
	Genode::Attached_rom_dataspace config { env, "config" };
	Genode::String<256> server_address = config.xml().attribute_value("server_address", Genode::String<256>("0.0.0.0:8899"));
	Libc::with_libc([&] () {
		RunServer(server_address.string());
	});

	return nullptr;
}

class Grpc_server::Server_main
{
	private:
		Env& _env;
		pthread_t _t;

	public:
		Server_main(Env& env)
			: _env(env)
		{
			pthread_create(&_t, 0, start_func, &env);
		}
};

void Libc::Component::construct(Libc::Env &env)
{
	static Grpc_server::Server_main main(env);
}
