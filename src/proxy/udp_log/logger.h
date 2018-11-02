/*
 * \brief  Network logger
 * \author Johannes Schlatow
 * \date   2018-11-01
 */

/*
 * Copyright (C) 2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <base/log.h>
#include <util/xml_node.h>

#include <net/udp.h>
#include <nic/packet_allocator.h>
#include <nic_session/connection.h>

using namespace Net;

namespace Udp_log {
	using Genode::size_t;
	using Genode::Xml_node;
	using Nic::Packet_stream_source;
	using Nic::Packet_descriptor;

	class Payload;
	template <typename MSG, typename PREFIX> class Logger;
};

class Udp_log::Payload
{
	private:
		char             _data[0];

	public:

		void set(char const *p, size_t plen, char const *s, size_t len, Size_guard &size_guard)
		{
			/* write prefix */
			size_guard.consume_head(plen);
			Genode::memcpy(_data, p, plen);

			/* write string */
			size_guard.consume_head(len);
			Genode::memcpy(&_data[plen], s, len);

			/* zero-out unconsumed data */
			size_t const unconsumed = size_guard.unconsumed();
			size_guard.consume_head(unconsumed);
			Genode::memset(&_data[plen+len], 0, unconsumed);
		}
};

template <typename MSG, typename PREFIX>
class Udp_log::Logger
{
	private:
		enum {
			PACKET_SIZE = 512,
			BUF_SIZE = Nic::Session::QUEUE_SIZE * PACKET_SIZE
		};

		Ipv4_address const _default_ip_address  { (Genode::uint8_t)0x00 };

		Nic::Packet_allocator _tx_block_alloc;
		Nic::Connection       _nic;

		Mac_address  _src_mac { _nic.mac_address() };
		Ipv4_address _src_ip;
		Port const   _src_port { 51234 };

		bool         _verbose { false };
		bool         _chksum_offload { false };

		Genode::Signal_handler<Logger> _source_ack;
		Genode::Signal_handler<Logger> _source_submit;

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

		Packet_stream_source< ::Nic::Session::Policy> * source() {
			return _nic.tx(); }

	public:
		Logger(Genode::Env &env, Genode::Allocator &alloc, Xml_node config)
			:
			 _tx_block_alloc(&alloc),
			 _nic(env, &_tx_block_alloc, BUF_SIZE, BUF_SIZE),
			 _src_ip (config.attribute_value("src_ip",  _default_ip_address)),
			 _verbose(config.attribute_value("verbose", _verbose)),
			 _chksum_offload(config.attribute_value("chksum_offload", _chksum_offload)),
			 _source_ack(env.ep(), *this, &Logger::_ready_to_ack),
			 _source_submit(env.ep(), *this, &Logger::_packet_avail)
		{
			_nic.tx_channel()->sigh_ack_avail(_source_ack);
			_nic.tx_channel()->sigh_ready_to_submit(_source_submit);
		}

		size_t write(PREFIX const &prefix, MSG const &string,
		             Ipv4_address const &ipaddr,
		             Port         const &port,
		             Mac_address  const &mac)
		{
			if (!_nic.link_state()) {
				return 0;
			}

			enum {
				HDR_SZ      = sizeof(Ethernet_frame) + sizeof(Ipv4_packet) + sizeof(Udp_packet),
				MIN_DATA_SZ = Ethernet_frame::MIN_SIZE - HDR_SZ,
			};

			size_t const packet_size = HDR_SZ + Genode::max((size_t)MIN_DATA_SZ,
			                                                string.size() + prefix.length());

			try {

				/* copy and submit packet */
				Packet_descriptor packet  = source()->alloc_packet(packet_size);
				Size_guard        size_guard(packet_size);
				void             *base    = source()->packet_content(packet);

				/* create ETH header */
				Ethernet_frame &eth = Ethernet_frame::construct_at(base, size_guard);
				eth.dst(mac);
				eth.src(_src_mac);
				eth.type(Ethernet_frame::Type::IPV4);

				/* create IP header */
				enum { IPV4_TIME_TO_LIVE = 64 };
				size_t const ip_off = size_guard.head_size();
				Ipv4_packet &ip = eth.construct_at_data<Ipv4_packet>(size_guard);
				ip.header_length(sizeof(Ipv4_packet) / 4);
				ip.version(4);
				ip.time_to_live(IPV4_TIME_TO_LIVE);
				ip.protocol(Ipv4_packet::Protocol::UDP);
				ip.src(_src_ip);
				ip.dst(ipaddr);

				/* create UDP header */
				size_t const udp_off = size_guard.head_size();
				Udp_packet &udp = ip.construct_at_data<Udp_packet>(size_guard);
				udp.src_port(_src_port);
				udp.dst_port(port);

				/* write payload */
				Payload &payload = udp.construct_at_data<Payload>(size_guard);
				payload.set(prefix.string(), prefix.length()-1,
				            string.string(), string.size(), size_guard);

				/* fill in header values that need the packet to be complete already */
				udp.length(size_guard.head_size() - udp_off);
				if (!_chksum_offload)
					udp.update_checksum(ip.src(), ip.dst());
				ip.total_length(size_guard.head_size() - ip_off);
				ip.update_checksum();

				source()->submit_packet(packet);
			} catch(Packet_stream_source<Nic::Session::Policy>::Packet_alloc_failed) {
				Genode::warning("Packet dropped");
			}

			if (_verbose)
				Genode::log(prefix, Genode::Cstring(string.string(), string.size()-2));

			return string.size();
		}

		
};
