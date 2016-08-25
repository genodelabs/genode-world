/*
 * \brief  TODO
 * \author Johannes Schlatow
 * \date   2016-02-18
 */

#include <base/env.h>
#include <base/exception.h>
#include <base/log.h>

#include <backend_base.h>

#include <nic/packet_allocator.h>
#include <nic_session/connection.h>

#include <net/ethernet.h>
#include <net/ipv4.h>

#include <os/config.h>

namespace Remote_rom {
	bool verbose = false;
	using  Genode::size_t;
	using  Genode::uint16_t;
	using  Genode::uint32_t;
	using  Genode::Cstring;
	using  Genode::Packet_descriptor;
	using  Genode::env;
	using  Net::Ethernet_frame;
	using  Net::Ipv4_packet;
	using  Net::Mac_address;
	using  Net::Ipv4_address;

	template <class>
	class  Backend_base;
	class  Backend_server;
	class  Backend_client;

	struct Packet_base;
	struct SignalPacket;
	struct UpdatePacket;
	struct DataPacket;
};

/* Packet format we use for inter-system communication */
class Remote_rom::Packet_base : public Ethernet_frame, public Ipv4_packet
{
	public:
		enum {
			MAX_NAME_LEN = 64        /* maximum length of the module name */
		};

		typedef enum {
			SIGNAL    = 1,           /* signal that ROM content has changed     */
			UPDATE    = 2,           /* request transmission of updated content */
			DATA      = 3,           /* first data packet                       */
			DATA_CONT = 4,           /* following data packets                  */
		} Type;

	protected:
		char         _module_name[MAX_NAME_LEN];   /* the ROM module name */
		Type         _type;                        /* packet type */
		uint32_t     _content_size;                /* ROM content size in bytes */
		uint32_t     _offset;                      /* offset in bytes */
		uint16_t     _payload_size;                /* payload size in bytes */

		/*****************************************************
		 ** 'payload' must be the last member of this class **
		 *****************************************************/

		char payload[0];

	public:

		Packet_base(size_t size) 
		:
			Ethernet_frame(sizeof(Packet_base) + size),
			Ipv4_packet(sizeof(Packet_base) + size - sizeof(Ethernet_frame)),
			_payload_size(size)
		{ }

		void const * base() const { return &payload; }

		/**
		 * Return type of the packet
		 */
		Type type() const { return _type; }

		/**
		 * Return size of the packet
		 */
		size_t size() const { return _payload_size + sizeof(Packet_base); }

		/**
		 * Return content_size of the packet
		 */
		size_t content_size() const { return _content_size; }

		/**
		 * Return offset of the packet
		 */
		size_t offset() const { return _offset; }

		/**
		 * Return module_name of the packet
		 */
		char* module_name() { return _module_name; }

		/**
		 * Set payload size of the packet
		 */
		void payload_size(Genode::size_t payload_size)
		{
			_payload_size = payload_size;
			Ipv4_packet::total_length(size() - sizeof(Ethernet_frame));
		}

		/**
		 * Return payload size of the packet
		 */
		size_t payload_size() const { return _payload_size; }

		/**
		 * Return address of the payload
		 */
		void *addr() { return payload; }

		void prepare_ethernet(const Mac_address &src, const Mac_address &dst=Ethernet_frame::BROADCAST)
		{
			Ethernet_frame::src(src);
			Ethernet_frame::dst(dst);
			Ethernet_frame::type(IPV4);
		}

		void prepare_ipv4(const Ipv4_address &src, const Ipv4_address &dst=Ipv4_packet::BROADCAST)
		{
			Ipv4_packet::version(4);
			Ipv4_packet::header_length(5);
			Ipv4_packet::time_to_live(10);
			Ipv4_packet::src(src);
			Ipv4_packet::dst(dst);
			Ipv4_packet::total_length(size() - sizeof(Ethernet_frame));
		}

		void set_checksums()
		{
			Ipv4_packet::checksum(Ipv4_packet::calculate_checksum(*this));
		}

		/**
		 * Placement new.
		 */
		void * operator new(Genode::size_t size, void* addr) {
			return addr; }

} __attribute__((packed));

class Remote_rom::SignalPacket : public Packet_base
{
	public:
		SignalPacket() : Packet_base(0)
		{ }

		void prepare(const char *module)
		{
			Genode::strncpy(_module_name, module, MAX_NAME_LEN);
			_type = SIGNAL;
			_payload_size = 0;
		}
} __attribute__((packed));

class Remote_rom::UpdatePacket : public Packet_base
{
	public:
		UpdatePacket() : Packet_base(0)
		{ }

		void prepare(const char *module)
		{
			Genode::strncpy(_module_name, module, MAX_NAME_LEN);
			_type = UPDATE;
			_payload_size = 0;
		}
} __attribute__((packed));

class Remote_rom::DataPacket : public Packet_base
{
	public:
		enum { MAX_PAYLOAD_SIZE = 1024 };

		char payload[MAX_PAYLOAD_SIZE];

		DataPacket() : Packet_base(MAX_PAYLOAD_SIZE)
		{ }

		void prepare(const char* module, size_t offset, size_t content_size)
		{
			Genode::strncpy(_module_name, module, MAX_NAME_LEN);

			_payload_size = MAX_PAYLOAD_SIZE;
			_offset       = offset;
			_content_size = content_size;

			if (offset == 0)
				_type = DATA;
			else
				_type = DATA_CONT;
		}

		/**
		 * Return packet size for given payload 
		 */
		static size_t packet_size(size_t payload) { return sizeof(Packet_base) + Genode::min(payload, MAX_PAYLOAD_SIZE); }

} __attribute__((packed));

template <class HANDLER>
class Remote_rom::Backend_base
{
	protected:
		enum {
			PACKET_SIZE = 1024,
			BUF_SIZE = Nic::Session::QUEUE_SIZE * PACKET_SIZE
		};

		class Rx_thread : public Genode::Thread
		{
			protected:
				Ipv4_address    &_accept_ip;
				Nic::Connection &_nic;
				HANDLER         &_handler;

				Genode::Signal_receiver              _sig_rec;
				Genode::Signal_dispatcher<Rx_thread> _link_state_dispatcher;
				Genode::Signal_dispatcher<Rx_thread> _rx_packet_avail_dispatcher;
				Genode::Signal_dispatcher<Rx_thread> _rx_ready_to_ack_dispatcher;

				void _handle_rx_packet_avail(unsigned)
				{
					while (_nic.rx()->packet_avail() && _nic.rx()->ready_to_ack()) {
						Packet_descriptor _rx_packet = _nic.rx()->get_packet();

						char *content = _nic.rx()->packet_content(_rx_packet);

						/* check IP */
						Ipv4_packet &ip_packet = *(Packet_base*)content;
						if (_accept_ip == Ipv4_packet::BROADCAST || _accept_ip == ip_packet.dst())
							_handler.receive(*(Packet_base*)content);

						_nic.rx()->acknowledge_packet(_rx_packet);
					}
				}

				void _handle_rx_ready_to_ack(unsigned) { _handle_rx_packet_avail(0); }

				void _handle_link_state(unsigned)
				{
					Genode::log("link state changed");
				}

			public:
				Rx_thread(Nic::Connection &nic, HANDLER &handler, Ipv4_address &ip)
				: Genode::Thread(Weight::DEFAULT_WEIGHT, "backend_nic_rx", 8192),
				  _accept_ip(ip),
				  _nic(nic), _handler(handler),
				  _link_state_dispatcher(_sig_rec, *this, &Rx_thread::_handle_link_state),
				  _rx_packet_avail_dispatcher(_sig_rec, *this, &Rx_thread::_handle_rx_packet_avail),
				  _rx_ready_to_ack_dispatcher(_sig_rec, *this, &Rx_thread::_handle_rx_ready_to_ack)
				{
					_nic.link_state_sigh(_link_state_dispatcher);
					_nic.rx_channel()->sigh_packet_avail(_rx_packet_avail_dispatcher);
					_nic.rx_channel()->sigh_ready_to_ack(_rx_ready_to_ack_dispatcher);
				} 

				void entry()
				{
					while(true)
					{
						Genode::Signal sig = _sig_rec.wait_for_signal();
						int num    = sig.num();

						Genode::Signal_dispatcher_base *dispatcher;
						dispatcher = dynamic_cast<Genode::Signal_dispatcher_base *>(sig.context());
						dispatcher->dispatch(num);
					}
				}
		};

		Nic::Packet_allocator _tx_block_alloc;
		Nic::Connection       _nic;
		Rx_thread             _rx_thread;
		Mac_address           _mac_address;
		Ipv4_address          _src_ip;
		Ipv4_address          _accept_ip;
		Ipv4_address          _dst_ip;

	protected:
		void _tx_ack(bool block = false)
		{
			/* check for acknowledgements */
			while (_nic.tx()->ack_avail() || block) {
				Nic::Packet_descriptor acked_packet = _nic.tx()->get_acked_packet();
				_nic.tx()->release_packet(acked_packet);
				block = false;
			}
		}

	public:
		explicit Backend_base(Genode::Allocator &alloc, HANDLER &handler)
		:
			_tx_block_alloc(&alloc), _nic(&_tx_block_alloc, BUF_SIZE, BUF_SIZE),
			_rx_thread(_nic, handler, _accept_ip)
		{
			/* start dispatcher thread */
			_rx_thread.start();

			/* store mac address */
			_mac_address = _nic.mac_address();

			try {
				char ip_string[15];
				Genode::Xml_node remoterom = Genode::config()->xml_node().sub_node("remote_rom");
				remoterom.attribute("src").value(ip_string, sizeof(ip_string));
				_src_ip = Ipv4_packet::ip_from_string(ip_string);

				remoterom.attribute("dst").value(ip_string, sizeof(ip_string));
				_dst_ip = Ipv4_packet::ip_from_string(ip_string);

				_accept_ip = _src_ip;
			} catch (...) {
				Genode::warning("No IP configured, falling back to broadcast mode!");
				_src_ip = Ipv4_packet::CURRENT;
				_dst_ip = Ipv4_packet::BROADCAST;
				_accept_ip = Ipv4_packet::BROADCAST;
			}
		}

		Nic::Packet_descriptor alloc_tx_packet(Genode::size_t size)
		{
			while (true) {
				try {
					Nic::Packet_descriptor packet = _nic.tx()->alloc_packet(size);
					return packet;
				} catch(Nic::Session::Tx::Source::Packet_alloc_failed) {
					/* packet allocator exhausted, wait for acknowledgements */
					_tx_ack(true);
				}
			}
		}

		void submit_tx_packet(Nic::Packet_descriptor packet)
		{
			_nic.tx()->submit_packet(packet);
			/* check for acknowledgements */
			_tx_ack();
		}
};

class Remote_rom::Backend_server : public Backend_server_base, public Backend_base<Backend_server>
{
	private:
		static Backend_server_base* _instance;
		Rom_forwarder_base         *_forwarder;

		Backend_server(Genode::Allocator &alloc) : Backend_base(alloc, *this), _forwarder(nullptr)
		{	}

		void send_data()
		{
			if (!_forwarder) return;

			size_t offset = 0;
			size_t size = _forwarder->content_size();
			while (offset < size)
			{
				/* create and transmit packet via NIC session */
				Nic::Packet_descriptor pd = alloc_tx_packet(DataPacket::packet_size(size));
				DataPacket *packet = new (_nic.tx()->packet_content(pd)) DataPacket();

				packet->prepare_ethernet(_mac_address, Ethernet_frame::BROADCAST);
				packet->prepare_ipv4(_src_ip, _dst_ip);
				packet->prepare(_forwarder->module_name(), offset, size);

				packet->payload_size(_forwarder->transfer_content((char*)packet->addr(), DataPacket::MAX_PAYLOAD_SIZE, offset));
				packet->set_checksums();

				submit_tx_packet(pd);

				offset += packet->payload_size();
			}
		}

	public:
		static Backend_server_base &instance()
		{
			if (!_instance) {
				_instance = new (env()->heap()) Backend_server(*env()->heap());

				if (!_instance)
					throw Exception();
			}

			return *_instance;
		}

		void register_forwarder(Rom_forwarder_base *forwarder)
		{
			_forwarder = forwarder;
		}

		void send_update()
		{
			if (!_forwarder) return;

			/* create and transmit packet via NIC session */
			Nic::Packet_descriptor pd = alloc_tx_packet(sizeof(SignalPacket));
			SignalPacket *packet = new (_nic.tx()->packet_content(pd)) SignalPacket();

			packet->prepare_ethernet(_mac_address);
			packet->prepare_ipv4(_src_ip, _dst_ip);
			packet->prepare(_forwarder->module_name());
			packet->set_checksums();

			submit_tx_packet(pd);
		}

		void receive(Packet_base &packet)
		{
			switch (packet.type())
			{
				case Packet_base::UPDATE:
					if (verbose)
						Genode::log("receiving UPDATE (", Cstring(packet.module_name()), ") packet");

					if (!_forwarder)
						return;

					/* check module name */
					if (Genode::strcmp(packet.module_name(), _forwarder->module_name()))
						return;

					/* TODO (optional) dont send data within Rx_Thread's context */
					send_data();
					
					break;
				default:
					break;
			}
		}
};

class Remote_rom::Backend_client : public Backend_client_base, public Backend_base<Backend_client>
{
	private:
		static Backend_client_base *_instance;
		Rom_receiver_base          *_receiver;
		char                       *_write_ptr;
		size_t                     _buf_size;

		Backend_client(Genode::Allocator &alloc) : Backend_base(alloc, *this), _receiver(nullptr), _write_ptr(nullptr), _buf_size(0)
		{
		}

		void write(char *data, size_t offset, size_t size)
		{
			if (!_write_ptr) return;

			size_t const len = Genode::min(size, _buf_size-offset);
			Genode::memcpy(_write_ptr+offset, data, len);

			if (offset + len >= _buf_size)
				_receiver->commit_new_content();
		}

	public:
		static Backend_client_base &instance()
		{
			if (!_instance) {
				_instance = new (env()->heap()) Backend_client(*env()->heap());

				if (!_instance)
					throw Exception();
			}

			return *_instance;
		}

		void register_receiver(Rom_receiver_base *receiver)
		{
			/* TODO support multiple receivers (ROM names) */
			_receiver = receiver;

			/* FIXME request update on startup (occasionally triggers invalid signal-context capability) */
//			if (_receiver)
//				update(_receiver->module_name());
		}


		void update(const char* module_name)
		{
			if (!_receiver) return;

			/* check module name */
			if (Genode::strcmp(module_name, _receiver->module_name()))
				return;

			/* create and transmit packet via NIC session */
			Nic::Packet_descriptor pd = alloc_tx_packet(sizeof(UpdatePacket));
			UpdatePacket *packet = (UpdatePacket*)_nic.tx()->packet_content(pd);

			packet->prepare_ethernet(_mac_address);
			packet->prepare_ipv4(_src_ip, _dst_ip);
			packet->prepare(_receiver->module_name());
			packet->set_checksums();

			submit_tx_packet(pd);
		}

		void receive(Packet_base &packet)
		{
			switch (packet.type())
			{
				case Packet_base::SIGNAL:
					if (verbose)
						Genode::log("receiving SIGNAL(", Cstring(packet.module_name()), ") packet");

					/* send update request */
					update(packet.module_name());
					
					break;
				case Packet_base::DATA:
					if (verbose)
						Genode::log("receiving DATA(", Cstring(packet.module_name()), ") packet");

					/* write into buffer */
					if (!_receiver) return;

					/* check module name */
					if (Genode::strcmp(packet.module_name(), _receiver->module_name()))
						return;
					
					_write_ptr = _receiver->start_new_content(packet.content_size());
					_buf_size  = (_write_ptr) ? packet.content_size() : 0;

					write((char*)packet.addr(), packet.offset(), packet.payload_size());
					
					break;
				case Packet_base::DATA_CONT:
					if (verbose)
						Genode::log("receiving DATA_CONT(", Cstring(packet.module_name()), ") packet");

					if (!_receiver) return;

					/* check module name */
					if (Genode::strcmp(packet.module_name(), _receiver->module_name()))
						return;

					/* write into buffer */
					write((char*)packet.addr(), packet.offset(), packet.payload_size());
					
					break;
				default:
					break;
			}
		}
};

Remote_rom::Backend_server_base *Remote_rom::Backend_server::_instance = nullptr;
Remote_rom::Backend_client_base *Remote_rom::Backend_client::_instance = nullptr;

Remote_rom::Backend_server_base &Remote_rom::backend_init_server()
{
	return Backend_server::instance();
}

Remote_rom::Backend_client_base &Remote_rom::backend_init_client()
{
	return Backend_client::instance();
}
