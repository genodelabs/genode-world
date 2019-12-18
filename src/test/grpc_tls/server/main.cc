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

static void *start_func(void*)
{
	Libc::with_libc([&] () {
		RunServer();
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
