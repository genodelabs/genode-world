/*
 * \brief  UDP log receiver
 * \author Johannes Schlatow
 * \date   2018-11-02
 */

/*
 * Copyright (C) 2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <net/udp.h>
#include <net/arp.h>

#include <base/log.h>
#include <util/xml_node.h>

#include <nic/packet_allocator.h>
#include <nic_session/connection.h>

using namespace Net;

namespace Log_udp {
	using Genode::size_t;
	using Genode::Xml_node;
	using Nic::Packet_stream_sink;
	using Nic::Packet_stream_source;
	using Nic::Packet_descriptor;

	class Receiver;
};

class Log_udp::Receiver
{
	private:
		enum {
			PACKET_SIZE = 512,
			BUF_SIZE    = Nic::Session::QUEUE_SIZE * PACKET_SIZE,
		};

		Nic::Packet_allocator _tx_block_alloc;
		Nic::Connection       _nic;

		Mac_address  const    _mac        { _nic.mac_address() };
		Ipv4_address const    _default_ip { (Genode::uint8_t)0x00 };
		Ipv4_address const    _ip;
		Port const   _default_port { 9 };
		Port const   _port;
		bool         _verbose { false };

		Genode::Signal_handler<Receiver> _sink_ack;
		Genode::Signal_handler<Receiver> _sink_submit;
		Genode::Signal_handler<Receiver> _source_ack;
		Genode::Signal_handler<Receiver> _source_submit;

		/**
		 * submit queue not empty anymore
		 */
		void _ready_to_submit();

		/**
		 * acknowledgement queue not full anymore
		 *
		 * we assume ACK and SUBMIT queue to be equally
		 * dimensioned so that this signal can be ignored.
		 */
		void _ack_avail() { }

		/**
		 * acknowledgement queue not empty anymore
		 */
		void _ready_to_ack()
		{
			/* check for acknowledgements */
			while (source()->ack_avail())
				source()->release_packet(source()->get_acked_packet());
		}

		/**
		 * submit queue not full anymore
		 *
		 * by now, packets are dropped if submit queue is full
		 */
		void _packet_avail() { }


		Packet_stream_sink< ::Nic::Session::Policy> * sink() {
			return _nic.rx(); }

		Packet_stream_source< ::Nic::Session::Policy> * source() {
			return _nic.tx(); }

	public:
		Receiver(Genode::Env &env, Genode::Allocator &alloc, Xml_node config)
			:
			 _tx_block_alloc(&alloc),
			 _nic(env, &_tx_block_alloc, BUF_SIZE, BUF_SIZE),
			 _ip     (config.attribute_value("ip",   _default_ip)),
			 _port   (config.attribute_value("port", _default_port)),
			 _verbose(config.attribute_value("verbose", _verbose)),
			 _sink_ack     (env.ep(), *this, &Receiver::_ack_avail),
			 _sink_submit  (env.ep(), *this, &Receiver::_ready_to_submit),
			 _source_ack   (env.ep(), *this, &Receiver::_ready_to_ack),
			 _source_submit(env.ep(), *this, &Receiver::_packet_avail)
		{
			_nic.rx_channel()->sigh_ready_to_ack(_sink_ack);
			_nic.rx_channel()->sigh_packet_avail(_sink_submit);
			_nic.tx_channel()->sigh_ack_avail(_source_ack);
			_nic.tx_channel()->sigh_ready_to_submit(_source_submit);
		}

		/**
		 * Handle an ethernet packet
		 *
		 * \param src   ethernet frame's address
		 * \param size  ethernet frame's size.
		 */
		void handle_ethernet(void* src, Genode::size_t size);

		/*
		 * Handle an ARP packet
		 *
		 * \param eth   ethernet frame containing the ARP packet.
		 * \param size  size guard
		 */
		void handle_arp(Ethernet_frame &eth,
		                Size_guard     &size_guard);

		/*
		 * Handle an IP packet
		 *
		 * \param eth   ethernet frame containing the IP packet.
		 * \param size  size guard
		 */
		void handle_ip(Ethernet_frame &eth,
		               Size_guard     &size_guard);

		/*
		 * Handle a LOG message packet
		 *
		 * \param udp   UDP packet containing the LOG message.
		 * \param size  size guard
		 */
		void handle_message(Udp_packet &udp,
		                    Size_guard &size_guard);

		/**
		 * Send ethernet frame
		 *
		 * \param eth   ethernet frame to send.
		 * \param size  ethernet frame's size.
		 */
		void send(Ethernet_frame *eth, Genode::size_t size);

};

void Log_udp::Receiver::_ready_to_submit()
{
	Packet_descriptor _packet { };

	/* as long as packets are available, and we can ack them */
	while (sink()->packet_avail()) {
		_packet = sink()->get_packet();
		if (_packet.size()) {
			handle_ethernet(sink()->packet_content(_packet), _packet.size());
		}

		if (!sink()->ready_to_ack()) {
			Genode::warning("ack state FULL");
			return;
		}

		sink()->acknowledge_packet(_packet);
	}
}

void Log_udp::Receiver::handle_ethernet(void* src, Genode::size_t size)
{
	try {
		/* parse ethernet frame header */
		Size_guard size_guard(size);
		Ethernet_frame &eth = Ethernet_frame::cast_from(src, size_guard);
		switch (eth.type()) {
		case Ethernet_frame::Type::ARP:
			handle_arp(eth, size_guard);
			break;
		case Ethernet_frame::Type::IPV4:
			handle_ip(eth, size_guard);
			break;
		default:
			;
		}
	} catch(Size_guard::Exceeded) {
		Genode::warning("Packet size guard exceeded!");
	}
}

void Log_udp::Receiver::handle_arp(Ethernet_frame &eth,
                                   Size_guard     &size_guard)
{
	/* ignore broken packets */
	Arp_packet &arp = eth.data<Arp_packet>(size_guard);
	if (!arp.ethernet_ipv4())
		return;

	/* look whether the IP address is our's */
	if (arp.dst_ip() == _ip) {
		if (arp.opcode() == Arp_packet::REQUEST) {
			/*
			 * The ARP packet gets re-written, we interchange source
			 * and destination MAC and IP addresses, and set the opcode
			 * to reply, and then push the packet back to the NIC driver.
			 */
			Ipv4_address old_src_ip = arp.src_ip();
			arp.opcode(Arp_packet::REPLY);
			arp.dst_mac(arp.src_mac());
			arp.src_mac(_mac);
			arp.src_ip(arp.dst_ip());
			arp.dst_ip(old_src_ip);
			eth.dst(arp.dst_mac());

			/* set our MAC as sender */
			eth.src(_mac);
			send(&eth, size_guard.total_size());
		}
	}
}

void Log_udp::Receiver::handle_ip(Ethernet_frame &eth,
                                  Size_guard     &size_guard)
{
	Ipv4_packet &ip = eth.data<Ipv4_packet>(size_guard);
	if (ip.protocol() == Ipv4_packet::Protocol::UDP) {

		Udp_packet &udp = ip.data<Udp_packet>(size_guard);
		if (udp.dst_port() == Port(9))
			handle_message(udp, size_guard);
	}
}

void Log_udp::Receiver::handle_message(Udp_packet &udp,
                                       Size_guard &size_guard)
{
	char *msg = &udp.data<char>(size_guard);

	/* consume until '\0' */
	size_t len = 0;
	for (char* s=msg; s && *s; s++, len++) {
		if (len)
			size_guard.consume_head(1);
	}

	if (len)
		Genode::log(Genode::Cstring(msg, len-1));
}

void Log_udp::Receiver::send(Ethernet_frame *eth, Genode::size_t size)
{
	try {
		/* copy and submit packet */
		Packet_descriptor packet  = source()->alloc_packet(size);
		char             *content = source()->packet_content(packet);
		Genode::memcpy((void*)content, (void*)eth, size);
		source()->submit_packet(packet);
	} catch(Packet_stream_source< ::Nic::Session::Policy>::Packet_alloc_failed) {
		Genode::warning("Packet dropped");
	}
}
