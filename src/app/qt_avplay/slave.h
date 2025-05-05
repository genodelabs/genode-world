/*
 * \brief  Convenience helper for running a service as child process
 * \author Norman Feske
 * \author Christian Helmuth
 * \date   2012-01-25
 */

/*
 * Copyright (C) 2012-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__OS__SLAVE_H_
#define _INCLUDE__OS__SLAVE_H_

/* Genode includes */
#include <base/rpc_server.h>
#include <base/local_connection.h>
#include <base/child.h>
#include <init/child_policy.h>
#include <os/child_policy_dynamic_rom.h>
#include <os/session_requester.h>

namespace Genode { struct Slave; }


struct Genode::Slave
{
	typedef Session_state::Args Args;

	class Policy;

	template <typename>
	class Connection_base;

	template <typename>
	struct Connection;
};


class Genode::Slave::Policy : public Child_policy
{
	public:

		typedef Child_policy::Name Name;
		typedef Session_label      Label;

		typedef Registered<Genode::Parent_service> Parent_service;
		typedef Registry<Parent_service>           Parent_services;

		Pd_session                   &ref_pd;
		Pd_session_capability   const ref_pd_cap;

	private:

		Label                   const _label;
		Binary_name             const _binary_name;
		Genode::Parent_service        _binary_service;
		Cap_quota               const _cap_quota;
		Ram_quota               const _ram_quota;
		Parent_services              &_parent_services;
		Rpc_entrypoint               &_ep;
		Ram_allocator                &_env_ram;
		Child_policy_dynamic_rom_file _config_policy;
		Session_requester             _session_requester;

		void _with_matching_service(Service::Name const &service_name,
		                            Session_label const &label,
		                            auto const &fn, auto const &denied_fn)
		{
			/* check for config file request */
			if (Service *s = _config_policy.resolve_session_request(service_name, label)) {
				fn(*s);
				return;
			}

			if (service_name == "ROM") {
				Session_label const rom_name(label.last_element());
				if (rom_name == _binary_name) {
					fn(_binary_service);
					return;
				}
				if (rom_name == "session_requests") {
					fn(_session_requester.service());
					return;
				}
			}

			/* fill parent service registry on demand */
			Parent_service *service = nullptr;
			_parent_services.for_each([&] (Parent_service &s) {
				if (!service && s.name() == service_name)
					service = &s; });

			if (!service) {
				error(name(), ": illegal session request of "
				      "service \"", service_name, "\" (", label, ")");
				denied_fn();
				return;
			}
			fn(*service);
		}

	public:

		class Connection;

		/**
		 * Slave-policy constructor
		 *
		 * \param ep        entrypoint used to provide local services
		 *                  such as the config ROM service
		 *
		 * \throw Out_of_ram   by 'Child_policy_dynamic_rom_file'
		 * \throw Out_of_caps  by 'Child_policy_dynamic_rom_file'
		 */
		Policy(Env                  &env,
		       Label          const &label,
		       Name           const &binary_name,
		       Parent_services      &parent_services,
		       Rpc_entrypoint       &ep,
		       Cap_quota             cap_quota,
		       Ram_quota             ram_quota)
		:
			ref_pd(env.pd()), ref_pd_cap(env.pd_session_cap()),
			_label(label), _binary_name(binary_name),
			_binary_service(env, Rom_session::service_name()),
			_cap_quota(cap_quota), _ram_quota(ram_quota),
			_parent_services(parent_services), _ep(ep), _env_ram(env.ram()),
			_config_policy(env.rm(), "config", _ep, &env.ram()),
			_session_requester(ep, env.ram(), env.rm())
		{
			configure("<config/>");
		}

		/**
		 * Assign new configuration to slave
		 *
		 * \param config  new configuration as null-terminated string
		 */
		void configure(char const *config)
		{
			_config_policy.load(config, strlen(config) + 1);
		}

		void configure(char const *config, size_t len)
		{
			_config_policy.load(config, len);
		}

		void trigger_session_requests()
		{
			_session_requester.trigger_update();
		}


		/****************************
		 ** Child_policy interface **
		 ****************************/

		Name name() const override { return _label; }

		Binary_name binary_name() const override { return _binary_name; }

		Ram_allocator &session_md_ram() override { return _env_ram; }

		Pd_account            &ref_account()           override { return ref_pd; }
		Capability<Pd_account> ref_account_cap() const override { return ref_pd_cap; }

		void init(Pd_session &session, Pd_session_capability cap) override
		{
			session.ref_account(ref_pd_cap);
			ref_pd.transfer_quota(cap, _cap_quota);
			ref_pd.transfer_quota(cap, _ram_quota);
		}

		void _with_route(Service::Name     const &name,
		                 Session_label     const &label,
		                 Session::Diag     const  diag,
		                 With_route::Ft    const &fn,
		                 With_no_route::Ft const &denied_fn) override
		{
			_with_matching_service(name, label,
				[&] (Service &service) { fn(Route { .service = service,
				                                    .label   = label,
				                                    .diag    = diag }); },
				denied_fn);
		}

		Id_space<Parent::Server> &server_id_space() override {
			return _session_requester.id_space(); }
};


template <typename CONNECTION>
class Genode::Slave::Connection_base
{
	protected:

		/* each connection appears as a separate client */
		Id_space<Parent::Client> _id_space;

		Policy &_policy;

		struct Service : Genode::Service, Session_state::Ready_callback,
		                                  Session_state::Closed_callback
		{
			Policy   &_policy;
			Blockade  _blockade { };
			bool      _alive = false;

			Service(Policy &policy)
			:
				Genode::Service(CONNECTION::service_name()), _policy(policy)
			{ }

			void initiate_request(Session_state &session) override
			{
				switch (session.phase) {

				case Session_state::CREATE_REQUESTED:

					if (!session.id_at_server.constructed())
						session.id_at_server.construct(session,
						                               _policy.server_id_space());

					session.ready_callback = this;
					session.async_client_notify = true;
					break;

				case Session_state::UPGRADE_REQUESTED:
					warning("upgrading slaves is not implemented");
					session.phase = Session_state::CAP_HANDED_OUT;
					break;

				case Session_state::CLOSE_REQUESTED:
					warning("closing slave connections is not implemented");
					session.phase = Session_state::CLOSED;
					break;

				case Session_state::SERVICE_DENIED:
				case Session_state::INSUFFICIENT_RAM_QUOTA:
				case Session_state::INSUFFICIENT_CAP_QUOTA:
				case Session_state::AVAILABLE:
				case Session_state::CAP_HANDED_OUT:
				case Session_state::CLOSED:
					break;
				}
			}

			void wakeup() override { }

			/**
			 * Session_state::Ready_callback
			 */
			void session_ready(Session_state &session) override
			{
				_alive = session.alive();
				_blockade.wakeup();
			}

			/**
			 * Session_state::Closed_callback
			 */
			void session_closed(Session_state &s) override { _blockade.wakeup(); }

			/**
			 * Service ('Ram_transfer::Account') interface
			 */
			void transfer(Pd_session_capability to, Ram_quota amount) override
			{
				if (to.valid()) _policy.ref_account().transfer_quota(to, amount);
			}

			/**
			 * Service ('Ram_transfer::Account') interface
			 */
			Pd_session_capability cap(Ram_quota) const override
			{
				return _policy.ref_pd_cap;
			}

			/**
			 * Service ('Cap_transfer::Account') interface
			 */
			void transfer(Pd_session_capability to, Cap_quota amount) override
			{
				if (to.valid()) _policy.ref_account().transfer_quota(to, amount);
			}

			/**
			 * Service ('Cap_transfer::Account') interface
			 */
			Pd_session_capability cap(Cap_quota) const override
			{
				return _policy.ref_pd_cap;
			}

		} _service;

		Local_connection<CONNECTION> _connection;

		Connection_base(Policy &policy, Args const &args, Affinity const &affinity)
		:
			_policy(policy), _service(_policy),
			_connection(_service, _id_space, { 1 }, args, affinity)
		{
			_policy.trigger_session_requests();
			_service._blockade.block();
			if (!_service._alive)
				throw Service_denied();
		}

		~Connection_base()
		{
			_policy.trigger_session_requests();
			_service._blockade.block();
		}

		typedef typename CONNECTION::Session_type SESSION;

		Capability<SESSION> _cap() const { return _connection.cap(); }
};


template <typename CONNECTION>
struct Genode::Slave::Connection : private Connection_base<CONNECTION>,
                                   public  CONNECTION::Client
{
	/**
	 * Constructor
	 *
	 * \throw Service_denied   parent denies session request
	 * \throw Out_of_ram       our own quota does not suffice for
	 *                         the creation of the new session
	 * \throw Out_of_caps
	 */
	Connection(Slave::Policy &policy, Args const &args,
	           Affinity const &affinity = Affinity())
	:
		Connection_base<CONNECTION>(policy, args, affinity),
		CONNECTION::Client(Connection_base<CONNECTION>::_cap())
	{ }

	Connection(Region_map &rm, Slave::Policy &policy, Args const &args,
	           Affinity const &affinity = Affinity())
	:
		Connection_base<CONNECTION>(policy, args, affinity),
		CONNECTION::Client(rm, Connection_base<CONNECTION>::_cap())
	{ }
};

#endif /* _INCLUDE__OS__SLAVE_H_ */
