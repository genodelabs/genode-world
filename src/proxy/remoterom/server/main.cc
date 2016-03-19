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

#include <backend_base.h>

#include <os/server.h>
#include <os/config.h>
#include <os/attached_rom_dataspace.h>

namespace Remoterom {
	using Genode::size_t;
	using Genode::Attached_rom_dataspace;

	class Rom_forwarder;
	struct Main;

	static char modulename[255];
	static char remotename[255];
	static bool binary = false;

};

struct Remoterom::Rom_forwarder : Rom_forwarder_base
{
		Attached_rom_dataspace &_rom;
		Backend_server_base    &_backend;

		Rom_forwarder(Attached_rom_dataspace &rom, Backend_server_base &backend) : _rom(rom), _backend(backend)
		{
			_backend.register_forwarder(this);

			/* on startup, send an update message to remote client */
			update(0);
		}

		const char *module_name() const { return remotename; }

		void update(unsigned)
		{
			/* TODO don't update ROM if a transfer is still in progress */

			/* refresh dataspace if valid*/
			_rom.update();

			/* trigger backend_server */
			_backend.send_update();
		}

		size_t content_size() const
		{
			if (_rom.is_valid()) {
				if (binary)
					return _rom.size();
				else
					return Genode::min(1+Genode::strlen(_rom.local_addr<char>()), _rom.size());
			}
			else {
				try {
					Genode::Xml_node default_content = Genode::config()->xml_node().sub_node("remoterom").sub_node("default");
					return default_content.content_size();
				} catch (...) { }
			}
			return 0;
		}

		size_t transfer_content(char *dst, size_t dst_len, size_t offset=0) const
		{
			if (_rom.is_valid()) {
				size_t const len = Genode::min(dst_len, content_size()-offset);
				Genode::memcpy(dst, _rom.local_addr<char>() + offset, len);
				return len;
			}
			else {
				/* transfer default content if set */
				try {
					Genode::Xml_node default_content = Genode::config()->xml_node().sub_node("remoterom").sub_node("default");
					size_t const len = Genode::min(dst_len, default_content.content_size()-offset);
					Genode::memcpy(dst, default_content.content_base() + offset, len);
					return len;
				} catch (...) { }
			}
			
			return 0;
		}
};

struct Remoterom::Main
{
	Server::Entrypoint    &_ep;
	Attached_rom_dataspace _rom;
	Rom_forwarder          _forwarder;

	Genode::Signal_rpc_member<Rom_forwarder> _dispatcher =
		{ _ep, _forwarder, &Rom_forwarder::update };

	Main(Server::Entrypoint &ep) : _ep(ep), _rom(modulename), _forwarder(_rom, backend_init_server())
	{
		/* register update dispatcher */
		_rom.sigh(_dispatcher);
	}
};

namespace Server {
	char const * name()            { return "remoterom_srv_ep"; }
	Genode::size_t stack_size()    { return 2*1024*sizeof(long); }
	void construct(Entrypoint &ep)
	{
		try {
			Genode::Xml_node remoterom = Genode::config()->xml_node().sub_node("remoterom");
			remoterom.attribute("localname").value(Remoterom::modulename, sizeof(Remoterom::modulename));
			remoterom.attribute("name").value(Remoterom::remotename, sizeof(Remoterom::remotename));
			try {
				remoterom.attribute("binary").value(&Remoterom::binary);
			} catch (...) { }
		} catch (...) {
			PERR("No ROM module configured!");
		}

		static Remoterom::Main main(ep);
	}
}
