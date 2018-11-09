/*
 * \brief Client implementation
 * \author Johannes Schlatow
 * \author Edgard Schmidt
 * \date   2018-11-06
 */

#include <base.h>
#include <backend_base.h>

namespace Remote_rom {
	using  Genode::Cstring;
	using  Genode::Microseconds;

	class Content_receiver;
	class Backend_client;
};

class Remote_rom::Content_receiver
{
	private:
		enum {
			MAX_PAYLOAD_SIZE = DataPacket::MAX_PAYLOAD_SIZE,
			TIMEOUT_DATA_US   = 50000   /*  50ms */
		};

		char                      *_write_ptr      { nullptr };
		size_t                     _buf_size       { 0 };
		size_t                     _offset         { 0 };

		/* window state */
		size_t                     _window_id      { 0 };
		size_t                     _window_length  { 0 };
		size_t                     _next_packet_id { 0 };
		bool                       _omit_nack      { false };

		/* timeouts and general object management*/
		Timer::One_shot_timeout<Content_receiver> _timeout;
		Backend_client            &_backend;
		Rom_receiver_base         *_frontend       { nullptr };

		size_t _write_offset()
		{ return _offset + _next_packet_id * MAX_PAYLOAD_SIZE; }

		void timeout_handler(Genode::Duration);

		/* Noncopyable */
		Content_receiver(Content_receiver const &);
		Content_receiver &operator=(Content_receiver const &);

		bool _start_window(size_t window_length)
		{
			/* calculate offset to beginning of new window */
			_offset += _window_length * MAX_PAYLOAD_SIZE;

			/* zero packet_id marks first window */
			if (_next_packet_id > 0)
				_window_id++;

			_window_length  = window_length;
			_next_packet_id = 0;

			if (_offset >= _buf_size)
				return false;


			return true;
		}

		void _write(const void *data, size_t packet_id, size_t size)
		{
			if (!_write_ptr) return;
			if (packet_id != _next_packet_id) return;

			size_t const offset = _write_offset();
			if (offset >= _buf_size)
				return;

			size_t const len = Genode::min(size, _buf_size-offset);
			Genode::memcpy(_write_ptr+offset, data, len);
		}

	public:
		Content_receiver(Timer::Connection &timer,
		                 Backend_client    &backend)
		: _timeout(timer, *this, &Content_receiver::timeout_handler),
		  _backend(backend)
		{ }

		void register_receiver(Rom_receiver_base *frontend)
		{
			_frontend = frontend;
		}

		void start_new_content(unsigned hash,
		                       size_t   size)
		{
			if (!_frontend) return;

			_write_ptr      = _frontend->start_new_content(hash, size);
			_buf_size       = _write_ptr ? size : 0;
			_offset         = 0;
			_window_id      = 0;
			_window_length  = 0;
			_next_packet_id = 0;
			_omit_nack      = false;

			if (_timeout.scheduled())
				_timeout.discard();
		}

		/**********************
		 * frontend accessors *
		 **********************/

		unsigned content_hash()   const
		{ return _frontend ? _frontend->content_hash() : 0; }

		char const *module_name() const
		{ return _frontend ? _frontend->module_name()  : ""; }

		size_t content_size() const
		{ return _buf_size; }

		bool window_complete()
		{
			/* remark: also returns true if we havent started any window yet */
			return _next_packet_id == _window_length;
		}

		bool complete()
		{
			return _write_offset() >= _buf_size;
		}

		bool accept_packet(const DataPacket &p);

		size_t window_id()        { return _window_id; }
		size_t ack_until()        { return _next_packet_id; }
};

class Remote_rom::Backend_client :
  public Backend_client_base,
  public Backend_base
{
	private:
		friend class Content_receiver;

		Content_receiver   _content_receiver { _timer, *this };

		Backend_client(Backend_client &);
		Backend_client &operator= (Backend_client &);

		void update(const char* module_name)
		{
			/* check module name */
			if (Genode::strcmp(module_name, _content_receiver.module_name()))
				return;

			if (_verbose)
				Genode::log("sending UPDATE(", _content_receiver.module_name(), ")");

			transmit_notification(Packet::UPDATE, _content_receiver);
		}

		void send_ack(Content_receiver const &recv);

		void receive(Packet &packet, Size_guard &size_guard);

	public:

		Backend_client(Genode::Env &env,
		               Genode::Allocator &alloc,
		               Genode::Xml_node config,
		               Genode::Xml_node policy)
		: Backend_base(env, alloc, config, policy)
		{ }


		void register_receiver(Rom_receiver_base *receiver)
		{
			/* TODO support multiple receivers (ROM names) */
			_content_receiver.register_receiver(receiver);

			/*
			 * FIXME request update on startup
			 * (occasionally triggers invalid signal-context capability)
			 * */
//			if (_receiver)
//				update(_receiver->module_name());
		}
};

namespace Remote_rom {
	using Genode::Env;
	using Genode::Allocator;
	using Genode::Xml_node;

	Backend_client_base &backend_init_client(Env &env,
	                                         Allocator &alloc,
	                                         Xml_node config)
	{
		static Backend_client backend(env,
		                              alloc,
		                              config,
		                              config.sub_node("remote_rom"));
		return backend;
	}
};

void Remote_rom::Backend_client::send_ack(Content_receiver const &recv)
{
	size_t const frame_size = sizeof(Ethernet_frame)
	                        + sizeof(Ipv4_packet)
	                        + sizeof(Udp_packet)
	                        + sizeof(Packet)
	                        + sizeof(AckPacket);
	Nic::Packet_descriptor pd = alloc_tx_packet(frame_size);
	Size_guard size_guard(pd.size());

	char *content = _nic.tx()->packet_content(pd);
	Ethernet_frame &eth = prepare_eth(content, size_guard);

	size_t const ip_off = size_guard.head_size();
	Ipv4_packet    &ip  = prepare_ipv4(eth, size_guard);

	size_t const udp_off = size_guard.head_size();
	Udp_packet     &udp = prepare_udp(ip, size_guard);

	Packet &pak = udp.construct_at_data<Packet>(size_guard);
	pak.type(Packet::ACK);
	pak.module_name(recv.module_name());
	pak.content_hash(recv.content_hash());

	AckPacket &ack =
		pak.construct_at_data<AckPacket>(size_guard);
	ack.window_id(_content_receiver.window_id());
	ack.ack_until(_content_receiver.ack_until());

	/* fill in header values that need the packet to be complete already */
	udp.length(size_guard.head_size() - udp_off);
	if (!_chksum_offload)
		udp.update_checksum(ip.src(), ip.dst());

	ip.total_length(size_guard.head_size() - ip_off);
	ip.update_checksum();

	submit_tx_packet(pd);

	if (_verbose)
		Genode::log("Sent ACK for window ", _content_receiver.window_id());
}

void Remote_rom::Backend_client::receive(Packet     &packet,
                                         Size_guard &size_guard)
{
	switch (packet.type())
	{
		case Packet::SIGNAL:
		{
			const NotificationPacket &signal
				= packet.data<NotificationPacket>(size_guard);

			if (_verbose)
				Genode::log("receiving SIGNAL(",
						      Cstring(packet.module_name()),
						      ") packet, size ",
						      signal.content_size());

			/* start new content with given size and hash */
			_content_receiver.start_new_content(
					packet.content_hash(),
					signal.content_size());

			/* send update request */
			update(packet.module_name());

			break;
		}
		case Packet::DATA:
		{
			/* check module name */
			if (Genode::strcmp(packet.module_name(), _content_receiver.module_name()))
				return;

			/* check hash */
			if (packet.content_hash() != _content_receiver.content_hash()) {
				Genode::warning("ignoring hash mismatch ",
				                Genode::Hex(packet.content_hash()),
				                " != ",
				                Genode::Hex(_content_receiver.content_hash()));
				return;
			}

			const DataPacket &data = packet.data<DataPacket>(size_guard);
			size_guard.consume_head(data.payload_size());

			_content_receiver.accept_packet(data);

			break;
		}
		case Packet::UPDATE:
			/* drop UPDATE packets received from other clients */
			if (_verbose)
				Genode::log("ignoring UPDATE");
			break;
		default:
			Genode::error("unknown packet type (", Genode::Hex(packet.type()), ").");
			break;
	}
}

void Remote_rom::Content_receiver::timeout_handler(Genode::Duration)
{
	Genode::warning("timeout occurred waiting for packet ", _next_packet_id,
	                " in window ", _window_id, " of length ", _window_length);
	_backend.send_ack(*this);
}

bool Remote_rom::Content_receiver::accept_packet(const DataPacket &p)
{
	/**
	 * TODO replace return value with exceptions
	 */
	if (complete() || !_frontend) return false;

	if (_timeout.scheduled())
		_timeout.discard();

	if (window_complete()) {
		if (!_start_window(p.window_length())) {
			Genode::warning("unexpected error starting window of size ",
			                p.window_length());
			return false;
		}
	}

	/* drop packets with wrong window id */
	if (p.window_id() != _window_id) {
		if  (p.window_id() == _window_id-1
		  && p.packet_id() == p.window_length()-1) {

			/* rollback the window */
			_offset        -= _window_length * MAX_PAYLOAD_SIZE;
			_window_id      = p.window_id();
			_window_length  = p.window_length();
			_next_packet_id = _window_length;

			/* send acknowledge if we receive the last packet, as
			 * the sender missed our previous ACK */
			Genode::log("re-sending ACK");
			_backend.send_ack(*this);
		}
		return false;
	}

	/* NACK if we missed a packet */
	if (p.packet_id() != _next_packet_id) {

		if (!_omit_nack) {

			Genode::log("lost packet, sending NACK");
			_backend.send_ack(*this);

			/* omit any further NACKs until we received a correct
			 * package or a timeout occurred */
			_omit_nack = true;
		}

		return false;
	}
	else {
		_omit_nack = false;
	}

	_write(p.addr(), p.packet_id(), p.payload_size());
	_next_packet_id++;

	if (window_complete())
		_backend.send_ack(*this);

	if (complete())
		_frontend->commit_new_content();
	else
		_timeout.schedule(Microseconds(TIMEOUT_DATA_US));

	return true;
}
