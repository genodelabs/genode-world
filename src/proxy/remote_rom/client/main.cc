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

namespace Remote_rom {
	using Genode::size_t;
	using Genode::Constructible;
	using Genode::Signal_context_capability;

	class  Session_component;
	class  Root;
	struct Main;
	struct Read_buffer;

	typedef Genode::List_element<Session_component> Session_element;
	typedef Genode::List<Session_element>           Session_list;
};


/**
 * Interface used by the sessions to obtain the ROM data received from the
 * remote server
 */
struct Remote_rom::Read_buffer : Genode::Interface
{
	virtual size_t content_size() const = 0;
	virtual size_t export_content(char *dst, size_t dst_len) const = 0;
};

class Remote_rom::Session_component :
  public Genode::Rpc_object<Genode::Rom_session, Session_component>
{
	private:
		Genode::Env &_env;

		Signal_context_capability _sigh;

		Read_buffer const &_buffer;

		Session_list &_sessions;
		Session_element _element;

		Constructible<Genode::Attached_ram_dataspace> _ram_ds;

	public:

		static int version() { return 1; }

		Session_component(Genode::Env &env, Session_list &sessions,
		                  Read_buffer const &buffer)
		:
		  _env(env), _sigh(), _buffer(buffer), _sessions(sessions), _element(this),
		  _ram_ds()
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
			using namespace Genode;

			/* replace dataspace by new one as needed */
			if (!_ram_ds.is_constructed()
			 || _buffer.content_size() > _ram_ds->size()) {

				_ram_ds.construct(_env.ram(), _env.rm(), _buffer.content_size());
			}

			char             *dst = _ram_ds->local_addr<char>();
			size_t const dst_size = _ram_ds->size();

			/* fill with content of current evaluation result */
			size_t const copied_len = _buffer.export_content(dst, dst_size);

			/* clear remainder of dataspace */
			Genode::memset(dst + copied_len, 0, dst_size - copied_len);

			/* cast RAM into ROM dataspace capability */
			Dataspace_capability ds_cap = static_cap_cast<Dataspace>(_ram_ds->cap());
			return static_cap_cast<Rom_dataspace>(ds_cap);
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
		Read_buffer    &_buffer;
		Session_list    _sessions;

	protected:

		Session_component *_create_session(const char *)
		{
			using namespace Genode;

			/*
			 * TODO compare requested module name with provided module name (config)
			 * */

			return new (Root::md_alloc())
			            Session_component(_env, _sessions, _buffer);
		}

	public:

		Root(Genode::Env &env, Genode::Allocator &md_alloc, Read_buffer &buffer)
		:
		  Genode::Root_component<Session_component>(&env.ep().rpc_ep(), &md_alloc),
		  _env(env),
		  _buffer(buffer),
		  _sessions()
		{ }

		void notify_clients()
		{
			for (Session_element *s = _sessions.first(); s; s = s->next())
				s->object()->notify_client();
		}
};

struct Remote_rom::Main : public Read_buffer, public Rom_receiver_base
{
	Genode::Env &env;
	Genode::Heap heap   = { &env.ram(), &env.rm() };
	Root remote_rom_root = { env, heap, *this };

	Genode::Constructible<Genode::Attached_ram_dataspace> _ds;
	size_t                                                _ds_content_size;

	Genode::Attached_rom_dataspace _config = { env, "config" };

	Backend_client_base &_backend;

	char remotename[255];

	Main(Genode::Env &env) :
	  env(env), _ds(), _ds_content_size(1024),
	  _backend(backend_init_client(env, heap))
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

	const char* module_name() const { return remotename; }

	char* start_new_content(size_t len)
	{
		 /* Create buffer for new data */
		_ds_content_size = len;

		// TODO (optional) implement double buffering
		if (!_ds.is_constructed() || _ds_content_size > _ds->size())
			_ds.construct(env.ram(), env.rm(), _ds_content_size);

		// TODO set write lock

		return _ds->local_addr<char>();
	}

	void commit_new_content(bool abort=false)
	{
		if (abort)
			Genode::error("abort not supported");

		// TODO release write lock

		remote_rom_root.notify_clients();
	}

	size_t content_size() const
	{
		if (_ds.is_constructed()) {
			return _ds_content_size;
		}
		else {
			/* transfer default content if set */
			try {
				Genode::Xml_node default_content = _config.xml().
				                                   sub_node("remote_rom").
				                                   sub_node("default");
				return default_content.content_size();
			} catch (...) { }
		}

		return 0;
	}

	size_t export_content(char *dst, size_t dst_len) const
	{
		if (_ds.is_constructed()) {
			size_t const len = Genode::min(dst_len, _ds_content_size);
			Genode::memcpy(dst, _ds->local_addr<char>(), len);
			return len;
		}
		else {
			/* transfer default content if set */
			try {
				Genode::Xml_node default_content = _config.xml().
				                                   sub_node("remote_rom").
				                                   sub_node("default");
				size_t const len = Genode::min(dst_len,
				                               default_content.content_size());
				Genode::memcpy(dst, default_content.content_base(), len);
				return len;
			} catch (...) { }
		}
		return 0;
	}
};

namespace Component {
	Genode::size_t stack_size() { return 2*1024*sizeof(long); }
	void construct(Genode::Env &env)
	{
		static Remote_rom::Main main(env);
	}
}
