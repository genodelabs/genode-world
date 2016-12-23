/*
 * \brief  TFTP client, ROM server
 * \author Emery Hemingway
 * \date   2016-02-24
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <nic/packet_allocator.h>
#include <timer_session/connection.h>
#include <rom_session/rom_session.h>
#include <os/signal_rpc_dispatcher.h>
#include <os/session_policy.h>
#include <os/path.h>
#include <base/attached_rom_dataspace.h>
#include <base/heap.h>
#include <root/component.h>
#include <libc/component.h>
#include <util/list.h>
#include <util/string.h>
#include <util/endian.h>

/* lwIP raw API */
#include <lwip/genode.h>
#include <lwip/api.h>
#include <lwip/inet.h>
#include <lwip/udp.h>
#include <lwip/init.h>


namespace Tftp_rom {

	using namespace Genode;

	class Timeout_dispatcher;
	class Session_component;
	class Root;
	struct Main;

	typedef List<Session_component> Session_list;

}


extern "C" void rrq_cb(void *arg, udp_pcb *upcb, pbuf *pbuf,
                       ip_addr_t *addr, Genode::uint16_t port);

extern "C" void data_cb(void *arg, udp_pcb *upcb, pbuf *pbuf,
                        ip_addr_t *addr, Genode::uint16_t port);


class Tftp_rom::Session_component :
	public Genode::Rpc_object<Genode::Rom_session>,
	public Session_list::Element,
	Genode::Lock
{
	private:

		Genode::Env &_env;

		typedef Genode::String<128> Filename;
		Filename const _filename;

		Ram_dataspace_capability  _dataspace;
		Signal_context_capability _sigh;

		udp_pcb *_pcb; /* lwIP UDP context  */
		pbuf    *_chain_head = NULL;
		pbuf    *_chain_tail = NULL;
		/*
		 * References to both ends of the buffer chain
		 * are retained to make concatenation faster.
		 */

		unsigned long const _start; /* start of session */

		unsigned       _ack_timeout = 1 << 11;
		unsigned const _client_timeout;

		uint16_t       _block_num;  /* TFTP block number */

		ip_addr_t       _addr;
		uint16_t  const _port;

		inline void finalize()
		{
			unlock();
			_ack_timeout = 0;
			if (_chain_head != NULL) {
				pbuf_free(_chain_head);
				_chain_head = NULL;
			}
		}

		inline void timeout()
		{
			Genode::error(_filename.string(), " timed out");
			finalize();
		}

	public:

		void initial_request()
		{
			udp_bind(_pcb, IP_ADDR_ANY, 0);

			Genode::size_t filename_len = Genode::strlen(_filename.string());

			pbuf *req = pbuf_alloc(PBUF_TRANSPORT, filename_len+9, PBUF_RAM);

			uint8_t *buf = (uint8_t*)req->payload;
	
			buf[0] = 0x00;
			buf[1] = 0x01;

			Genode::strncpy((char*)buf+2, _filename.string(), filename_len+1);
			Genode::strncpy((char*)buf+3+filename_len, "octet", 6);

			udp_sendto(_pcb, req, &_addr, _port);
		}

		Session_component(Genode::Env  &env,
		                  char const   *namestr,
		                  ip_addr      &ipaddr,
		                  uint16_t      port,
		                  unsigned long now,
		                  unsigned      timeout)
		:
			Lock(LOCKED),
			_env(env),
			_filename(namestr),
			_pcb(udp_new()),
			_start(now),
			_client_timeout(timeout),
			_addr(ipaddr), _port(port)
		{
			if (_pcb == NULL) {
				Genode::error("failed to create UDP context");
				throw Genode::Root::Unavailable();
			}

			/* set callback */
			udp_recv(_pcb, rrq_cb, this);

			initial_request();
		}

		~Session_component()
		{
			using namespace Genode;

			if (_pcb != NULL)
				udp_remove(_pcb);

			if (_chain_head != NULL)
				pbuf_free(_chain_head);

			if (_dataspace.valid())
				_env.ram().free(_dataspace);
		}

		/**************************************
		 ** Members available to lwIP thread **
		 **************************************/

		ip_addr_t *addr() { return &_addr; }

		void send_ack()
		{
			pbuf    *ack = pbuf_alloc(PBUF_TRANSPORT, 4, PBUF_RAM);
			uint8_t *buf = (uint8_t*)ack->payload;

			buf[0] = 0x00;
			buf[1] = 0x04;

			buf[2] = _block_num >> 8;
			buf[3] = _block_num;

			udp_send(_pcb, ack);
		}

		void first_response(pbuf *data, ip_addr_t *addr, uint16_t port)
		{
			/*
			 * we now know the port the server will use,
			 * lwIP will now drop all other packets
			 */
			udp_connect(_pcb, addr, port);

			/* swap out the callback */
			udp_recv(_pcb, data_cb, this);
		}

		/**
		 * Returns false if data was not in
		 * response to the last request
		 */
		bool add_block(pbuf *data)
		{
			using Genode::size_t;

			uint8_t *buf = (uint8_t*)data->payload;

			/* TFTP packets always start with zero */
			if (buf[0])
				return false;

			if (buf[1] == 0x05) {
				buf[data->len-1] = '\0';
				Genode::error(_filename.string(), ": ", (const char *)buf+4);
				_ack_timeout = 0;
				/* permanent error, inform the client */
				finalize();
				pbuf_free(data);
				return true;
			}

			if ((buf[1] != 0x03)
			 || (host_to_big_endian(*((uint16_t*)buf+1)) != (_block_num+1)))
				return false;

			++_block_num;
			send_ack();

			bool done = data->len < 516;

			/* hit the hard 32MB limit */
			if (!done && _block_num == 0xffff) {
				Genode::error(_filename.string(), ": maximum file size exceded!");
				finalize();
			}

			if (_chain_head == NULL)
				_chain_head = _chain_tail = data;
			else {
				/* data pointer is invalid after pbuf_cat */
				pbuf_cat(_chain_tail, data);
				_chain_tail = _chain_tail->next;
			}

			if (done) /* construct the dataspace */ {

				size_t rom_len = 0;

				/*
				 * pbuf.tot_len is only a 16 bit number so
				 * a recount is probably required
				 */
				for (pbuf *link = _chain_head; link != NULL; link = link->next)
					rom_len += link->len-4;

				_dataspace = _env.ram().alloc(rom_len);
				uint8_t *rom_addr = _env.rm().attach(_dataspace);
				uint8_t *p = rom_addr;

				for (pbuf *link = _chain_head; link != NULL; link = link->next) {
					size_t len = link->len - 4;
					Genode::memcpy(p, ((uint8_t*)link->payload)+4, len);
					p += len;
				}

				_env.rm().detach(rom_addr);
				Genode::log(_filename.string(), " retrieved");
				finalize();
			}

			return true;
		}

		/*************************************
		 ** Tiggered by timer on RPC thread **
		 *************************************/

		bool done() const { return _ack_timeout == 0; }

		void check_time(unsigned long now)
		{
			/* XXX: timer rollover? */
			if (!_block_num) {
				if (_client_timeout & (_client_timeout < now - _start))
					timeout();
				else
	 				initial_request();
				return;
			}

			unsigned period = (now - _start) / _block_num;

			if (_client_timeout && (_client_timeout < period)) {
				timeout();
				return;
			}

			if (_ack_timeout < period)
				send_ack();

			_ack_timeout = period+(period/2);
		}

		/***************************
		 ** ROM session interface **
		 ***************************/

		Rom_dataspace_capability dataspace() override
		{
			if (!done()) lock();

			Dataspace_capability ds = _dataspace;
			return static_cap_cast<Genode::Rom_dataspace>(ds);
		};

		void sigh(Signal_context_capability sigh) override { _sigh = sigh; }

};


/********************
 ** lwIP callbacks **
 ********************/


extern "C" void rrq_cb(void *arg, udp_pcb *upcb, pbuf *data,
                       ip_addr_t *addr, Genode::uint16_t port)
{
	Tftp_rom::Session_component *session = (Tftp_rom::Session_component*)arg;

	if (!ip_addr_cmp(addr, session->addr())) {
		Genode::error("dropping rogue packet");
		pbuf_free(data);
		return;
	}

	if (session->add_block(data)) {
		session->first_response(data, addr, port);
		return;
	}

	pbuf_free(data);
	session->initial_request();
}


extern "C" void data_cb(void *arg, udp_pcb *upcb, pbuf *data,
                        ip_addr_t *addr, Genode::uint16_t port)
{
	Tftp_rom::Session_component *session = (Tftp_rom::Session_component*)arg;
	if (session->add_block(data)) return;

	/* bad packet */
	pbuf_free(data);
	session->send_ack();
}


class Tftp_rom::Root : public Genode::Root_component<Session_component>
{
	private:

		Genode::Env                    &_env;
		Genode::Attached_rom_dataspace  _config_rom { _env, "config" };

		class Timeout_dispatcher : Genode::Thread, Genode::Lock
		{
			private:

				enum { TIMER_PERIOD_US = 1 << 20 };

				Timer::Connection          _timer;
				Signal_receiver            _sig_rec;
				Signal_context             _sig_ctx;
				Signal_context_capability  _sig_cap;
				Session_list               _sessions;

			protected:

				void entry() override
				{
					for (Genode::Signal_context *ctx = _sig_rec.wait_for_signal().context();
					     ctx == &_sig_ctx;
					     ctx = _sig_rec.wait_for_signal().context())
					{
						Lock::Guard(*this);

						Session_component *session = _sessions.first();
						if (!session) {
							_timer.sigh(Signal_context_capability());
							continue;
						}

						unsigned long now = _timer.elapsed_ms();

						do {
							if (session->done()) {
								Session_component *old = session;
								session = old->next();
								_sessions.remove(old);
							} else {
								session->check_time(now);
								session = session->next();
							}
						} while (session);
					}
				}

			public:

				Timeout_dispatcher(Genode::Env &env)
				:
					Genode::Thread(env, "timeout_ep", 1024 * sizeof(Genode::addr_t)),
					_timer(env), _sig_cap(_sig_rec.manage(&_sig_ctx))
				{
					_timer.trigger_periodic(TIMER_PERIOD_US);
					start();
				}

				/* A destructor for style */
				~Timeout_dispatcher()
				{
					/* break entry loop */
					Genode::Signal_context tmp_ctx;
					Genode::Signal_transmitter(_sig_rec.manage(&tmp_ctx)).submit();
					join();
					_sig_rec.dissolve(&tmp_ctx);
					_sig_rec.dissolve(&_sig_ctx);
				}

				unsigned long elapsed_ms() { return _timer.elapsed_ms(); }

				void insert(Session_component *session)
				{
					Lock::Guard(*this);

					if (!_sessions.first())
						_timer.sigh(_sig_cap);

					_sessions.insert(session);
				}

				void remove(Session_component *session)
				{
					Lock::Guard(*this);

					_sessions.remove(session);
					/* timer will be stopped at the next signal */
				}

		} _timeout_dispatcher { _env } ;

	protected:

		Session_component *_create_session(const char *args) override
		{
			Session_component *session;

			_config_rom.update();

			ip_addr  ipaddr;
			unsigned port = 69;
			unsigned timeout = 0;

			Session_label const label = label_from_args(args);
			Session_label const rom_name = label.last_element();

			try {

				Session_policy policy(label, _config_rom.xml());

				try {
					char addr_str[53];
					policy.attribute("ip").value(addr_str, sizeof(addr_str));
					ipaddr_aton(addr_str, &ipaddr);
				} catch (...) {
					Genode::error(label.string(), ": 'ip' not specified in policy");
					throw Root::Unavailable();
				}

				try { policy.attribute("port").value(&port); }
				catch (...) { }

				try { policy.attribute("timeout").value(&timeout); }
				catch (...) { }

				try {
					Path<1024> path;

					policy.attribute("dir").value(path.base(), path.capacity());
					path.append("/");
					path.append(rom_name.string());

					session = new (md_alloc())
						Session_component(_env, path.base(), ipaddr, port,
						                  _timeout_dispatcher.elapsed_ms(), timeout*1000);
					Genode::log((char const *)path.base(), " requested");
				} catch (...) { /* no dir attribute */
					session = new (md_alloc())
						Session_component(_env, rom_name.string(), ipaddr, port,
						                  _timeout_dispatcher.elapsed_ms(), timeout*1000);
					Genode::log(label.string(), " requested");
				}

			} catch (Session_policy::No_policy_defined) {
				Genode::error("no policy for defined for ", label.string());
				throw Root::Unavailable();
			}

			_timeout_dispatcher.insert(session);
			return session;
		}

		void _destroy_session(Session_component *session) override
		{
			_timeout_dispatcher.remove(session);
			Genode::destroy(md_alloc(), session);
		}

	public:

		Root(Genode::Env &env, Genode::Allocator &md_alloc)
		:
			Genode::Root_component<Session_component>(&env.ep().rpc_ep(), &md_alloc),
			_env(env)
		{
			env.parent().announce(env.ep().manage(*this));
		}
};


void Libc::Component::construct(Genode::Env &env )
{
	static Genode::Sliced_heap sliced_heap(env.ram(), env.rm());
	static Tftp_rom::Root root(env, sliced_heap);
}

