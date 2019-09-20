#include <base/attached_rom_dataspace.h>
#include <libc/component.h>
#include <base/heap.h>
#include <base/log.h>
#include <base/thread.h>
#include "greeter_server.h"

enum { STACK_SIZE = 0xF000 };

namespace Tee_server {
	using namespace Genode;

	class Runner;
	class Server_main;
}

class Tee_server::Runner : public Thread
{
	public:
		Runner(Env& env)
			: Thread(env, "runner", STACK_SIZE)
		{
		}

		void entry() override
		{
			Libc::with_libc([] () {
				RunServer();
			});
		}
};

class Tee_server::Server_main
{
	private:
		Env& _env;
		Runner _runner { _env };

	public:
		Server_main(Env& env)
			: _env(env)
		{
			_runner.start();
		}
};

void Libc::Component::construct(Libc::Env &env)
{
	static Tee_server::Server_main main(env);
}
