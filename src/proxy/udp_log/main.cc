/*
 * \brief  LOG server that sends data via UDP.
 * \author Johannes Schlatow
 * \date   2018-11-01
 */

/*
 * Copyright (C) 2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <base/env.h>
#include <base/heap.h>
#include <base/log.h>

#include <net/udp.h>

#include <base/component.h>
#include <base/attached_rom_dataspace.h>

#include <log_session/log_session.h>
#include <root/component.h>

#include <os/session_policy.h>

#include "logger.h"

using namespace Net;

namespace Udp_log {
	using Genode::size_t;
	using Genode::Attached_rom_dataspace;
	using Genode::Xml_node;
	using Genode::Log_session;

	class  Session_component;
	class  Root;
	struct Main;
};

class Udp_log::Session_component : public Genode::Rpc_object<Log_session>
{
	public:
		typedef Genode::String<Genode::Session_label::capacity()+3> Prefix;

	private:

		Genode::Env           &_env;
		Logger<String,Prefix> &_logger;

		Prefix _prefix;

		Mac_address  const _default_mac_address { (Genode::uint8_t)0xff };
		Ipv4_address const _default_ip_address  { (Genode::uint8_t)0x00 };
		Port         const _default_port        { 9 };

		Mac_address  const _dst_mac;
		Ipv4_address const _dst_ip;
		Port         const _dst_port;

	public:

		Session_component(Genode::Env &env, Logger<String,Prefix> &logger,
		                  Genode::Session_label const &label,
		                  Xml_node policy)
		:
			_env(env), _logger(logger), _prefix("[", label.string(), "] "),
			_dst_mac (policy.attribute_value("mac",  _default_mac_address)),
			_dst_ip  (policy.attribute_value("ip",   _default_ip_address)),
			_dst_port(policy.attribute_value("port", _default_port))
		{ }

		/* LOG session implementation */

		void write(String const &string) override
		{
			/*  TODO implement ARP, i.e. do not send LOG packets until MAC is known
			 *       for this we need the following modifications:
			 *       - add optional gateway attribute to config to allow specifying the next hop
			 *         if the dst_ip is not the next hop
			 *       - implement an Arp_resolver class on root level that send ARP requests using
			 *         the logger and handles ARP responses
			 *       - ARP responses are delivered to all session components
			 *       - a session component stores a single address
			 *       - a session component stores uses the broadcast MAC until the address is resolved
			 */

			_logger.write(_prefix, string, _dst_ip, _dst_port, _dst_mac);
		}
		
};


class Udp_log::Root : public Genode::Root_component<Session_component>
{
	private:

		Genode::Env                &_env;
		Genode::Allocator          &_alloc;
		Attached_rom_dataspace      _config = { _env, "config" };
		Logger<Log_session::String,
		       Session_component::Prefix>
		                            _logger = { _env, _alloc, _config.xml() };

	protected:

		Session_component *_create_session(const char *args) override
		{
			using namespace Genode;

			try {
				Session_label const label = label_from_args(args);
				Session_policy policy(label, _config.xml());
				
				return new (Root::md_alloc())
				            Session_component(_env, _logger, label, policy);
			}
			catch (Session_policy::No_policy_defined) {
				Genode::warning("Missing policy.");
				return 0;
			}
			catch (Out_of_ram) {
				Genode::warning("insufficient 'ram_quota'");
				throw Insufficient_ram_quota();
			}
			catch (Out_of_caps) {
				Genode::warning("insufficient 'cap_quota'");
				throw Insufficient_cap_quota();
			}
		}

	public:

		Root(Genode::Env &env, Genode::Allocator &md_alloc)
		: Genode::Root_component<Session_component>(&env.ep().rpc_ep(), &md_alloc),
		  _env(env), _alloc(md_alloc)
		{ }
};

struct Udp_log::Main
{
	Genode::Env &env;
	Genode::Heap heap = { &env.ram(), &env.rm() };
	Root         root = { env, heap };

	Main(Genode::Env &env) : env(env)
	{
		env.parent().announce(env.ep().manage(root));
	}

};

namespace Component {
	Genode::size_t stack_size()    { return 2*1024*sizeof(long); }
	void construct(Genode::Env &env)
	{
		env.exec_static_constructors();
		static Udp_log::Main main(env);
	}
}
