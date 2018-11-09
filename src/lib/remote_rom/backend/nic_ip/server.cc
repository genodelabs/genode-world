/*
 * \brief Server implementation
 * \author Johannes Schlatow
 * \author Edgard Schmidt
 * \date   2018-11-06
 */

#include <base.h>
#include <backend_base.h>

namespace Remote_rom {
	using  Genode::Cstring;
	using  Genode::Microseconds;

	class Content_sender;
	class Backend_server;
};

class Remote_rom::Content_sender
{
	private:
		enum {
			MAX_PAYLOAD_SIZE = DataPacket::MAX_PAYLOAD_SIZE,
			MAX_WINDOW_SIZE  = 900,
			TIMEOUT_ACK_US   = 1000000   /*  1000ms */
		};

		/* total data size */
		size_t _data_size     { 0 };

		/* current window length */
		size_t _window_length { MAX_WINDOW_SIZE };

		/* current window id */
		size_t _window_id     { 0 };

		/* data offset of current window */
		size_t _offset        { 0 };

		/* current packed id */
		size_t _packet_id     { 0 };

		size_t _errors        { 0 };

		/* timeouts and general object management*/
		Timer::One_shot_timeout<Content_sender> _timeout;
		Backend_server            &_backend;
		Rom_forwarder_base        *_frontend       { nullptr };

		void timeout_handler(Genode::Duration)
		{
			Genode::warning("no ACK received for window ", _window_id);

			/* only handle  */
			if (_errors == 0) {
				/* go back to beginning of window */
				go_back(0);
				transmit(false);
			}
			else {
				reset();
				_frontend->finish_transmission();
				Genode::warning("transmission cancelled");
			}
			_errors++;
		}

		/* Noncopyable */
		Content_sender(Content_sender const &);
		Content_sender &operator=(Content_sender const &);

		static size_t _calculate_window_size(size_t size)
		{
			size_t const mod = size % MAX_PAYLOAD_SIZE;
			size_t const packets = size / MAX_PAYLOAD_SIZE + (mod ? 1 : 0);

			return Genode::min((size_t)MAX_WINDOW_SIZE, packets);
		}

		/**
		 * Go to next packet. Returns false if window is complete.
		 */
		inline bool _next_packet()
		{ return ++_packet_id < _window_length; }

		inline bool _window_complete() const
		{ return _packet_id == _window_length; }

		inline bool _transmission_complete() const
		{ return _offset >= _data_size; }
		/**
		 * Return absolute data offset of current packet.
		 */
		inline size_t _data_offset() const
		{ return _offset + _packet_id * MAX_PAYLOAD_SIZE; }


		/**
		 * Go to next window. Returns false if end of data was reached.
		 * TODO we may adapt the window size if retransmission occurred
		 */
		bool _next_window() {
			/* advance offset by data transmitted in the last window */
			_offset += _window_length * MAX_PAYLOAD_SIZE;
			if (_transmission_complete())
				return false;

			_window_id++;
			_window_length = _calculate_window_size(_data_size-_offset);
			_packet_id = 0;
			
			return true;
		}

	public:
		Content_sender(Timer::Connection &timer, Backend_server &backend)
		: _timeout(timer, *this, &Content_sender::timeout_handler),
		  _backend(backend)
		{ }

		void register_forwarder(Rom_forwarder_base *forwarder)
		{
			_frontend = forwarder;
		}

		void reset()
		{
			_offset        = 0;
			_packet_id     = 0;
			_window_id     = 0;
			_data_size     = 0;
			_window_length = 0;
			_errors        = 0;
		}

		bool transmitting() { return _packet_id > 0; }

		/**********************
		 * frontend accessors *
		 **********************/

		unsigned content_hash() const
		{ return _frontend ? _frontend->content_hash() : 0; }

		size_t content_size() const
		{ return _frontend ? _frontend->content_size() : 0; }

		char const *module_name() const
		{ return _frontend ? _frontend->module_name()  : ""; }

		size_t transfer_content(char* dst, size_t max_size) const
		{
			if (!_frontend) return 0;

			return _frontend->transfer_content(dst, max_size, _data_offset());
		}

		/************************
		 * transmission control *
		 ************************/

		bool transmit(bool restart);

		void go_back(size_t packet_id) { _packet_id = packet_id; }

		/*************************************
		 * accessors for packet construction *
		 *************************************/

		/**
		 * Return payload size of current packet.
		 */
		size_t payload_size() const
		{ return Genode::min(_data_size-_data_offset(),
		                     (size_t)MAX_PAYLOAD_SIZE); }

		size_t window_id()     const { return _window_id; }
		size_t window_length() const { return _window_length; }
		size_t packet_id()     const { return _packet_id; }
};

class Remote_rom::Backend_server :
  public Backend_server_base,
  public Backend_base
{
	private:

		friend class Content_sender;

		Content_sender              _content_sender { _timer, *this };

		Backend_server(Backend_server &);
		Backend_server &operator= (Backend_server &);

		void send_packet(Content_sender const &sender);

		void receive(Packet &packet, Size_guard &);

	public:

		Backend_server(Genode::Env &env,
		               Genode::Allocator &alloc,
		               Genode::Xml_node config,
		               Genode::Xml_node policy)
		: Backend_base(env, alloc, config, policy)
		{ }


		void register_forwarder(Rom_forwarder_base *forwarder)
		{ _content_sender.register_forwarder(forwarder); }


		void send_update()
		{
			if (!_content_sender.content_size()) return;

			if (_verbose)
				Genode::log("sending SIGNAL(", _content_sender.module_name(), ")");

			/* TODO re-send SIGNAL packet after a timeout */
			transmit_notification(Packet::SIGNAL, _content_sender);
		}
};

namespace Remote_rom {
	using Genode::Env;
	using Genode::Allocator;
	using Genode::Xml_node;

	Backend_server_base &backend_init_server(Env &env,
	                                         Allocator &alloc,
	                                         Xml_node config)
	{
		static Backend_server backend(env,
		                              alloc,
		                              config,
		                              config.sub_node("remote_rom"));
		return backend;
	}
};

void Remote_rom::Backend_server::send_packet(Content_sender const &sender)
{
	/* create and transmit packet via NIC session */
	size_t const max_payload = sender.payload_size();
	size_t const max_size = sizeof(Ethernet_frame)
		                   + sizeof(Ipv4_packet)
		                   + sizeof(Udp_packet)
		                   + sizeof(Packet)
		                   + sizeof(DataPacket)
		                   + max_payload;
	Nic::Packet_descriptor pd = alloc_tx_packet(max_size);
	Size_guard size_guard(pd.size());

	char* const content = _nic.tx()->packet_content(pd);
	Ethernet_frame &eth = prepare_eth(content, size_guard);

	size_t const ip_off = size_guard.head_size();
	Ipv4_packet    &ip  = prepare_ipv4(eth, size_guard);

	size_t const udp_off = size_guard.head_size();
	Udp_packet     &udp = prepare_udp(ip, size_guard);

	Packet &pak = udp.construct_at_data<Packet>(size_guard);
	pak.type(Packet::DATA);
	pak.module_name(sender.module_name());
	pak.content_hash(sender.content_hash());

	DataPacket &data = pak.construct_at_data<DataPacket>(size_guard);
	data.window_id(sender.window_id());
	data.window_length(sender.window_length());
	data.packet_id(sender.packet_id());

	size_guard.consume_head(max_payload);
	data.payload_size(sender.transfer_content((char*)data.addr(),
		                                       max_payload));

	/* fill in header values that need the packet to be complete already */
	udp.length(size_guard.head_size() - udp_off);
	if (!_chksum_offload)
		udp.update_checksum(ip.src(), ip.dst());

	ip.total_length(size_guard.head_size() - ip_off);
	ip.update_checksum();

	submit_tx_packet(pd);
}

void Remote_rom::Backend_server::receive(Packet &packet,
                                         Size_guard &size_guard)
{
	switch (packet.type())
	{
		case Packet::UPDATE:
			if (_verbose)
				Genode::log("receiving UPDATE (",
				            Cstring(packet.module_name()),
				            ") packet");

			/* check module name */
			if (Genode::strcmp(packet.module_name(), _content_sender.module_name()))
				return;

			/* compare content hash */
			if (packet.content_hash() != _content_sender.content_hash()) {
				if (_verbose)
					Genode::log("ignoring UPDATE with invalid hash");
				return;
			}

			if (_verbose) {
				Genode::log("Sending data of size ", _content_sender.content_size());
			}

			_content_sender.transmit(true);

			break;
		case Packet::SIGNAL:
			if (_verbose)
				Genode::log("ignoring SIGNAL");
			break;
		case Packet::DATA:
			if (_verbose)
				Genode::log("ignoring DATA");
			break;
		case Packet::ACK:
		{
			if (!_content_sender.transmitting())
				return;

			if (Genode::strcmp(packet.module_name(), _content_sender.module_name()))
				return;

			if (packet.content_hash() != _content_sender.content_hash()) {
				if (_verbose)
					Genode::warning("ignoring ACK with wrong hash");
				return;
			}

			AckPacket const &ack = packet.data<AckPacket>(size_guard);

			if (ack.window_id() != _content_sender.window_id()) {
				if (_verbose)
					Genode::warning("ignoring ACK with wrong window id");
				return;
			}

			if (ack.ack_until() < _content_sender.packet_id()) {
				if (_verbose)
					Genode::warning("Go back to packet id ", ack.ack_until());

				_content_sender.go_back(ack.ack_until());
			}

			_content_sender.transmit(false);

			break;
		}
		default:
			break;
	}
}

bool Remote_rom::Content_sender::transmit(bool restart)
{
	if (!_frontend) return false;

	if (_timeout.scheduled())
		_timeout.discard();

	_errors = 0;

	if (restart) {
		/* do not start if we are still transmitting */
		if (!_transmission_complete())
			return false;

		_frontend->start_transmission();
		reset();

		_data_size     = _frontend->content_size();
		_window_length = _calculate_window_size(_data_size);
	}
	else if (_window_complete()) {
		if (!_next_window()) {
			_frontend->finish_transmission();

			if (transmitting()) {
				reset();
				return true;
			}
			else
				return false;
		}
	}

	do {
		_backend.send_packet(*this);
	} while (_next_packet());

	/* set ACK timeout */
	_timeout.schedule(Microseconds(TIMEOUT_ACK_US));

	return false;
}
