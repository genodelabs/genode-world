/*
 * \brief  TODO
 * \author Johannes Schlatow
 * \date   2016-02-15
 */

/** 
 * __________    ________________                 _________________    __________
 * | server | -> |  remoterom   | -> (network) -> |   remoterom   | -> | client |
 * |        |    |    server    |                 |    client     |    |        |
 * ----------    ----------------                 -----------------    ----------
 */

#include <base/printf.h>
#include <base/env.h>
#include <util/volatile_object.h>
#include <util/list.h>

#include <base/rpc_server.h>
#include <root/component.h>

#include <os/attached_ram_dataspace.h>
#include <rom_session/rom_session.h>

#include <base/component.h>
#include <os/config.h>

#include <backend_base.h>

namespace Remoterom {
	using Genode::size_t;
	using Genode::Lazy_volatile_object;
	using Genode::Signal_context_capability;

	class  Session_component;
	class  Root;
	struct Main;
	struct Read_buffer;

	static char remotename[255];

	typedef Genode::List<Session_component> Session_list;
};


/**
 * Interface used by the sessions to obtain the ROM data received from the remote server
 */
struct Remoterom::Read_buffer
{
	virtual size_t content_size() const = 0;
	virtual size_t export_content(char *dst, size_t dst_len) const = 0;
};

class Remoterom::Session_component : public Genode::Rpc_object<Genode::Rom_session, Remoterom::Session_component>,
                                     public Session_list::Element
{
	private:
		Genode::Env &_env;

		Signal_context_capability _sigh;

		Read_buffer const &_buffer;

		Session_list &_sessions;

		Lazy_volatile_object<Genode::Attached_ram_dataspace> _ram_ds;

	public:

		static int version() { return 1; }

		Session_component(Genode::Env &env, Session_list &sessions, Read_buffer const &buffer)
		:
			_env(env), _buffer(buffer), _sessions(sessions)
		{
			_sessions.insert(this);
		}

		~Session_component() { _sessions.remove(this); }

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

				_ram_ds.construct(&_env.ram(), _buffer.content_size());
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


class Remoterom::Root : public Genode::Root_component<Session_component>
{
	private:

		Genode::Env    &_env;
		Read_buffer    &_buffer;
		Session_list    _sessions;

	protected:

		Session_component *_create_session(const char *args)
		{
			using namespace Genode;

			/* TODO compare requested module name with provided module name (config) */

			return new (Root::md_alloc())
			            Session_component(_env, _sessions, _buffer);
		}

	public:

		Root(Genode::Env &env, Genode::Allocator &md_alloc, Read_buffer &buffer)
		: Genode::Root_component<Session_component>(&env.ep().rpc_ep(), &md_alloc), _env(env), _buffer(buffer)
		{ }

		void notify_clients()
		{
			for (Session_component *s = _sessions.first(); s; s = s->next())
				s->notify_client();
		}
};

struct Remoterom::Main : public Remoterom::Read_buffer, public Remoterom::Rom_receiver_base
{
	Genode::Env &env;
	Genode::Heap heap   = { &env.ram(), &env.rm() };
	Root remoterom_root = { env, heap, *this };

	Genode::Lazy_volatile_object<Genode::Attached_ram_dataspace> _ds;
	size_t                                                       _ds_content_size;

	Backend_client_base &_backend;

	Main(Genode::Env &env) : env(env), _ds_content_size(1024), _backend(backend_init_client())
	{
		env.parent().announce(env.ep().manage(remoterom_root));

		/* initialise backend */
		_backend.register_receiver(this);
		
//		_ds.construct(Genode::env()->ram_session(), _ds_content_size);
	}

	const char* module_name() const { return remotename; }

	char* start_new_content(size_t len)
	{
		 /* Create buffer for new data */
		_ds_content_size = len;

		// TODO (optional) implement double buffering
		if (!_ds.is_constructed() || _ds_content_size > _ds->size())
			_ds.construct(&env.ram(), _ds_content_size);

		// TODO set write lock

		return _ds->local_addr<char>();
	}

	void commit_new_content(bool abort=false)
	{
		if (abort)
			PERR("abort not supported");

		// TODO release write lock

		remoterom_root.notify_clients();
	}

	size_t content_size() const
	{
		if (_ds.is_constructed()) {
			return _ds_content_size;
		}
		else {
			/* transfer default content if set */
			try {
				Genode::Xml_node default_content = Genode::config()->xml_node().sub_node("remoterom").sub_node("default");
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
				Genode::Xml_node default_content = Genode::config()->xml_node().sub_node("remoterom").sub_node("default");
				size_t const len = Genode::min(dst_len, default_content.content_size());
				Genode::memcpy(dst, default_content.content_base(), len);
				return len;
			} catch (...) { }
		}
		return 0;
	}
};

namespace Component {
	Genode::size_t stack_size()    { return 2*1024*sizeof(long); }
	void construct(Genode::Env &env)
	{
		try {
			Genode::Xml_node remoterom = Genode::config()->xml_node().sub_node("remoterom");
			remoterom.attribute("name").value(Remoterom::remotename, sizeof(Remoterom::remotename));
		} catch (...) {
			PERR("No ROM module configured!");
		}

		static Remoterom::Main main(env);
	}
}
