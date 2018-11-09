/*
 * \brief  Client-side proxy component translating a rom_session provided over a network into a local rom_session.
 * \author Johannes Schlatow
 * \date   2016-02-15
 * 
 * Usage scenario:
 * __________    ______________                 ______________    __________
 * | server | -> | remote_rom | -> (network) -> | remote_rom | -> | client |
 * |        |    |   server   |                 |   client   |    |        |
 * ----------    --------------                 --------------    ----------
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <base/env.h>
#include <base/heap.h>
#include <base/log.h>
#include <util/reconstructible.h>
#include <util/list.h>

#include <base/rpc_server.h>
#include <root/component.h>

#include <base/attached_ram_dataspace.h>
#include <base/attached_rom_dataspace.h>
#include <rom_session/rom_session.h>

#include <base/component.h>

#include <backend_base.h>
#include <util.h>

namespace Remote_rom {
	using Genode::size_t;
	using Genode::Attached_ram_dataspace;
	using Genode::Rom_dataspace_capability;
	using Genode::Signal_context_capability;

	class  Session_component;
	class  Root;
	struct Main;
	struct Rom_module;

	typedef Genode::List_element<Session_component> Session_element;
	typedef Genode::List<Session_element>           Session_list;
};


/**
 * Interface used by the sessions to obtain the ROM data received from the
 * remote server
 */
class Remote_rom::Rom_module
{
	private:
		Genode::Ram_allocator &_ram;
		Attached_ram_dataspace _fg; /* dataspace delivered to clients */
		Attached_ram_dataspace _bg; /* dataspace for receiving data */

		unsigned _bg_hash { 0 };
		size_t   _bg_size { 0 };

	public:
		Rom_module(Genode::Ram_allocator &ram, Genode::Env &env)
		: _ram(ram),
		  _fg(_ram, env.rm(), 0),
		  _bg(_ram, env.rm(), 4096)
		{ }

		Rom_dataspace_capability fg_dataspace() const
		{
			using namespace Genode;

			if (!_fg.size())
				return Rom_dataspace_capability();

			Dataspace_capability ds_cap = _fg.cap();
			return static_cap_cast<Rom_dataspace>(ds_cap);
		}

		/**
		 * Return pointer to buffer that is ready to be filled with data.
		 *
		 * Data is written into the background dataspace.
		 * Once it is ready, the 'commit_bg()' function is called.
		 */
		char* base(size_t size)
		{
			/* let background buffer grow if needed */
			if (_bg.size() < size)
				_bg.realloc(&_ram, size);

			_bg_size = size;
			return _bg.local_addr<char>();
		}

		/**
		 * Commit data contained in background dataspace
		 * (swap foreground and background dataspace)
		 */
		bool commit_bg()
		{
			if (_bg_hash != cksum(_bg.local_addr<char>(), _bg_size)) {
				Genode::error("checksum error");
				return false;
			}

			_fg.swap(_bg);
			return true;
		}

		unsigned hash() const { return _bg_hash; }
		void hash(unsigned v) { _bg_hash = v; }

};

class Remote_rom::Session_component :
  public Genode::Rpc_object<Genode::Rom_session, Session_component>
{
	private:
		Genode::Env &_env;

		Signal_context_capability _sigh;

		Rom_module const &_rom_module;

		Session_list &_sessions;
		Session_element _element;

	public:

		static int version() { return 1; }

		Session_component(Genode::Env &env, Session_list &sessions,
		                  Rom_module const &rom_module)
		:
		  _env(env), _sigh(), _rom_module(rom_module), _sessions(sessions), _element(this)
		{
			_sessions.insert(&_element);
		}

		~Session_component() { _sessions.remove(&_element); }

		void notify_client()
		{
			if (!_sigh.valid())
				return;

			Genode::Signal_transmitter(_sigh).submit();
		}

		/* ROM session implementation */

		Genode::Rom_dataspace_capability dataspace() override
		{
			return _rom_module.fg_dataspace();
		}
		
		void sigh(Genode::Signal_context_capability sigh) override
		{
			_sigh = sigh;
		}
};

class Remote_rom::Root : public Genode::Root_component<Session_component>
{
	private:

		Genode::Env    &_env;
		Rom_module     &_rom_module;
		Session_list    _sessions;

	protected:

		Session_component *_create_session(const char *)
		{
			using namespace Genode;

			/*
			 * TODO compare requested module name with provided module name (config)
			 * */

			return new (Root::md_alloc())
			            Session_component(_env, _sessions, _rom_module);
		}

	public:

		Root(Genode::Env &env, Genode::Allocator &md_alloc, Rom_module &rom_module)
		:
		  Genode::Root_component<Session_component>(&env.ep().rpc_ep(), &md_alloc),
		  _env(env),
		  _rom_module(rom_module),
		  _sessions()
		{ }

		void notify_clients()
		{
			for (Session_element *s = _sessions.first(); s; s = s->next())
				s->object()->notify_client();
		}
};

struct Remote_rom::Main : public Rom_receiver_base
{
	Genode::Env &env;
	Genode::Heap heap            { &env.ram(), &env.rm() };
	Rom_module   rom_module      { env.ram(), env };
	Root         remote_rom_root { env, heap, rom_module };

	Genode::Attached_rom_dataspace _config = { env, "config" };

	Backend_client_base &_backend;

	char remotename[255];

	Main(Genode::Env &env) :
	  env(env),
	  _backend(backend_init_client(env, heap, _config.xml()))
	{
		try {
			Genode::Xml_node remote_rom = _config.xml().sub_node("remote_rom");
			remote_rom.attribute("name").value(remotename, sizeof(remotename));
		} catch (...) {
			Genode::error("No ROM module configured!");
		}

		env.parent().announce(env.ep().manage(remote_rom_root));

		/* initialise backend */
		_backend.register_receiver(this);
	}

	const char* module_name()  const { return remotename; }
	unsigned    content_hash() const { return rom_module.hash(); }

	char* start_new_content(unsigned hash, size_t len)
	{
		/* save expected hash */
		/* TODO (optional) skip if we already have the same data */
		rom_module.hash(hash);

		return rom_module.base(len);
	}

	void commit_new_content(bool abort=false)
	{
		if (abort)
			return;

		if (rom_module.commit_bg())
			remote_rom_root.notify_clients();
	}

};

namespace Component {
	Genode::size_t stack_size() { return 2*1024*sizeof(long); }
	void construct(Genode::Env &env)
	{
		static Remote_rom::Main main(env);
	}
}
