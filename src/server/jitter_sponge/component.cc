/*
 * \brief  Jitter/Keccak entropy server
 * \author Emery Hemingway
 * \date   2018-11-06
 */

/*
 * Copyright (C) 2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* local includes */
#include "session_requests.h"

#include <terminal_session/connection.h>
#include <base/attached_ram_dataspace.h>
#include <base/heap.h>
#include <base/component.h>

#include <world/rdrand.h>

/* Jitterentropy includes */
#include <jitterentropy.h>

/* Keccak includes */
extern "C" {
#include <KeccakPRGWidth1600.h>
}


namespace Jitter_sponge {
	using namespace Genode;

	struct Generator;
	class  Session_component;
	struct Main;

	typedef Genode::Id_space<Session_component> Session_space;

	struct Collection_failure : Genode::Exception { };
}


struct Jitter_sponge::Generator
{
	KeccakWidth1600_SpongePRG_Instance sponge { };

	struct rand_data *jitter = nullptr;

	void die(char const *msg)
	{
		/* forget sponge state */
		KeccakWidth1600_SpongePRG_Forget(&sponge);
		Genode::error(msg);
		throw Exception();
	}

	Generator(Allocator &alloc)
	{
		jitterentropy_init(alloc);
		if (jent_entropy_init())
			die("jitterentropy library could not be initialized!");

		jitter = jent_entropy_collector_alloc(0, 0);
		if (!jitter)
			die("failed to allocate jitter entropy collector");

		if (KeccakWidth1600_SpongePRG_Initialize(&sponge, 254))
			die("failed to initialize sponge");
	}

	void mix()
	{
		/* mix at entry and exit of 'read', so 32 bytes are mixed between reads */

		if (Genode::Rdrand::supported()) {
			enum { RDRAND_COUNT = 4 };
			Genode::uint64_t buf[4];
			for (unsigned i = 0; i < RDRAND_COUNT; i++)
				buf[i] = Genode::Rdrand::random64();
			if (KeccakWidth1600_SpongePRG_Feed(&sponge, (unsigned char *)buf, sizeof(buf)))
				die("failed to feed sponge");
		} else {
			enum { MIX_BYTES = 16};
			char buf[MIX_BYTES];
			if (jent_read_entropy(jitter, buf, MIX_BYTES) != MIX_BYTES)
				die("jitter collection failed");

			if (KeccakWidth1600_SpongePRG_Feed(&sponge, (unsigned char *)buf, MIX_BYTES))
				die("failed to feed sponge");
		}
	}

	void fetch(unsigned char *buf, size_t n)
	{
		if (KeccakWidth1600_SpongePRG_Fetch(&sponge, buf, n))
			die("failed to fetch from sponge");
	}
};


class Jitter_sponge::Session_component final : public Genode::Rpc_object<Terminal::Session, Session_component>
{
	private:

		Session_component(Session_component const &);
		Session_component &operator = (Session_component const &);

		Session_space::Element _sessions_elem;

		Genode::Attached_ram_dataspace _io_buffer;

		Generator &_generator;

	public:

		Session_component(Genode::Env &env,
		                  Session_space &space,
		                  Session_space::Id id,
		                  Generator &generator)
		:
			_sessions_elem(*this, space, id),
			_io_buffer(env.ram(), env.rm(), 0x1000),
			_generator(generator)
		{ }

		Genode::Dataspace_capability _dataspace() {
			return _io_buffer.cap(); }

		Genode::size_t _read(Genode::size_t n)
		{
			_generator.mix();
			n = min(n, _io_buffer.size());
			_generator.fetch(_io_buffer.local_addr<unsigned char>(), n);
			_generator.mix();
			return n;
		}

		Genode::size_t read(void *, Genode::size_t) override { return 0; }

		Genode::size_t _write(Genode::size_t) { return 0; }
		Genode::size_t write(void const *, Genode::size_t) override { return 0; }

		Size size() override { return Size(0, 0); }

		bool avail() override { return true; }

		void connected_sigh(Genode::Signal_context_capability cap) override {
			Genode::Signal_transmitter(cap).submit(); }

		void read_avail_sigh(Genode::Signal_context_capability cap) override {
			Genode::Signal_transmitter(cap).submit(); }

		void size_changed_sigh(Genode::Signal_context_capability) override { }
};


struct Jitter_sponge::Main : Session_request_handler
{
	Genode::Env  &_env;
	Heap          _entropy_heap { _env.ram(), _env.rm() };
	Sliced_heap   _session_heap { _env.ram(), _env.rm() };
	Generator     _generator    { _entropy_heap };
	Session_space _sessions     { };

	void handle_session_create(Session_state::Name const &,
	                           Parent::Server::Id pid,
	                           Session_state::Args const &args) override
	{
		size_t ram_quota =
			Arg_string::find_arg(args.string(), "ram_quota").ulong_value(0);
		size_t session_size =
			max((size_t)4096, sizeof(Session_component));

		if (ram_quota < session_size)
			throw Insufficient_ram_quota();

		Session_space::Id id { pid.value };

		Session_component *session = new (_session_heap)
			Session_component(_env, _sessions, id, _generator);

		_env.parent().deliver_session_cap(pid, _env.ep().manage(*session));
		_generator.mix();
	}

	void handle_session_upgrade(Parent::Server::Id,
	                            Session_state::Args const &) override { }

	void handle_session_close(Parent::Server::Id pid) override
	{
		Session_space::Id id { pid.value };
		_sessions.apply<Session_component&>(
			id, [&] (Session_component &session)
		{
			_env.ep().dissolve(session);
			destroy(_session_heap, &session);
			_env.parent().session_response(pid, Parent::Session_response::CLOSED);
		});
	}

	Session_requests_rom _session_requests { _env, *this };

	Main(Genode::Env &env) : _env(env)
	{
		env.parent().announce("Terminal");

		/* process any requests that have already queued */
		_session_requests.schedule();
	}
};


void Component::construct(Genode::Env &env)
{
	static Jitter_sponge::Main inst(env);
}
