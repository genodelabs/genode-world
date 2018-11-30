/*
 * \brief  Common base for client and server
 * \author Johannes Schlatow
 * \author Edgard Schmidt
 * \date   2016-02-18
 */

#include <net/ethernet.h>
#include <net/arp.h>
#include <net/ipv4.h>
#include <net/udp.h>
#include <net/port.h>

#include <base/env.h>
#include <base/log.h>
#include <base/attached_rom_dataspace.h>

#include <nic/packet_allocator.h>
#include <nic_session/connection.h>

#include <timer_session/connection.h>

#include <packet.h>

#ifndef __INCLUDE__REMOTE_ROM__BASE_H_
#define __INCLUDE__REMOTE_ROM__BASE_H_

namespace Remote_rom {
	using  Genode::Packet_descriptor;
	using  Genode::env;
	using  Net::Ethernet_frame;
	using  Net::Ipv4_packet;
	using  Net::Mac_address;
	using  Net::Ipv4_address;
	using  Net::Size_guard;
	using  Net::Arp_packet;
	using  Net::Udp_packet;
	using  Net::Port;

	class  Backend_base;
};


class Remote_rom::Backend_base : public Genode::Interface
{
	private:

		Genode::Signal_handler<Backend_base> _link_state_handler;
		Genode::Signal_handler<Backend_base> _rx_packet_handler;

		Port                        _udp_port;
		Port                  const _src_port    { 51234 };

		/* rx packet handler */
		void _handle_rx_packet();

		void _handle_arp_request(Ethernet_frame &eth,
		                         Size_guard     &guard,
		                         Arp_packet     &arp);

		void _handle_link_state()
		{
			Genode::log("link state changed");
		}

		void _tx_ack(bool block = false)
		{
			/* check for acknowledgements */
			while (_nic.tx()->ack_avail() || block) {
				Nic::Packet_descriptor acked_packet = _nic.tx()->get_acked_packet();
				_nic.tx()->release_packet(acked_packet);
				block = false;
			}
		}

	protected:

		enum {
			PACKET_SIZE = 1600,
			BUF_SIZE = Nic::Session::QUEUE_SIZE * PACKET_SIZE
		};

		Timer::Connection     _timer;

		const bool            _verbose = false;
		bool                  _arp_waiting { false };
		Nic::Packet_allocator _tx_block_alloc;
		Nic::Connection       _nic;
		Mac_address           _mac_address;
		Mac_address           _dst_mac;
		Ipv4_address          _src_ip;
		Ipv4_address          _accept_ip;
		Ipv4_address          _dst_ip;
		bool                  _chksum_offload { false };

		/**
		 * Handle accepted network packet from the other side
		 */
		virtual void receive(Packet &packet, Size_guard &) = 0;

		inline void arp_request()
		{
			if (_dst_mac == Ethernet_frame::broadcast() && !_arp_waiting) {
				size_t const frame_size = sizeof(Ethernet_frame)
			                           + sizeof(Arp_packet);
				Nic::Packet_descriptor pd = alloc_tx_packet(frame_size);
				Size_guard size_guard(pd.size());

				char *content = _nic.tx()->packet_content(pd);
				Ethernet_frame &eth = Ethernet_frame::construct_at(content,
				                                                   size_guard);
				eth.src(_mac_address);
				eth.dst(Ethernet_frame::broadcast());
				eth.type(Ethernet_frame::Type::ARP);

				Arp_packet &arp = eth.construct_at_data<Arp_packet>(size_guard);
				arp.hardware_address_type(Arp_packet::ETHERNET);
				arp.protocol_address_type(Arp_packet::IPV4);
				arp.hardware_address_size(sizeof(Mac_address));
				arp.protocol_address_size(sizeof(Ipv4_address));
				arp.opcode(Arp_packet::REQUEST);
				arp.src_mac(_mac_address);
				arp.src_ip(_src_ip);
				arp.dst_mac(Ethernet_frame::broadcast());
				arp.dst_ip(_dst_ip);

				_arp_waiting = true;
				submit_tx_packet(pd);
			}
		}

		Ethernet_frame &prepare_eth(void *base, Size_guard &size_guard)
		{
			arp_request();

			Ethernet_frame &eth = Ethernet_frame::construct_at(base, size_guard);
			eth.src(_mac_address);
			eth.dst(_dst_mac);
			eth.type(Ethernet_frame::Type::IPV4);

			return eth;
		}

		Ipv4_packet &prepare_ipv4(Ethernet_frame &eth, Size_guard &size_guard)
		{
			Ipv4_packet &ip = eth.construct_at_data<Ipv4_packet>(size_guard);
			ip.version(4);
			ip.header_length(sizeof(Ipv4_packet) / 4);
			ip.time_to_live(10);
			ip.protocol(Ipv4_packet::Protocol::UDP);
			ip.src(_src_ip);
			ip.dst(_dst_ip);

			return ip;
		}

		Udp_packet &prepare_udp(Ipv4_packet &ip, Size_guard &size_guard)
		{
			Udp_packet &udp = ip.construct_at_data<Udp_packet>(size_guard);
			udp.src_port(_src_port);
			udp.dst_port(_udp_port);

			return udp;
		}

		template <typename T>
		void transmit_notification(Packet::Type type,
		                           T const &frontend)
		{
			size_t const frame_size = sizeof(Ethernet_frame)
			                        + sizeof(Ipv4_packet)
			                        + sizeof(Udp_packet)
			                        + sizeof(Packet)
			                        + sizeof(NotificationPacket);
			Nic::Packet_descriptor pd = alloc_tx_packet(frame_size);
			Size_guard size_guard(pd.size());

			char *content = _nic.tx()->packet_content(pd);
			Ethernet_frame &eth = prepare_eth(content, size_guard);

			size_t const ip_off = size_guard.head_size();
			Ipv4_packet    &ip  = prepare_ipv4(eth, size_guard);

			size_t const udp_off = size_guard.head_size();
			Udp_packet     &udp = prepare_udp(ip, size_guard);

			Packet &pak = udp.construct_at_data<Packet>(size_guard);
			pak.type(type);
			pak.module_name(frontend.module_name());
			pak.content_hash(frontend.content_hash());

			NotificationPacket &npak =
				pak.construct_at_data<NotificationPacket>(size_guard);
			npak.content_size(frontend.content_size());

			/* fill in header values that need the packet to be complete already */
			udp.length(size_guard.head_size() - udp_off);
			if (!_chksum_offload)
				udp.update_checksum(ip.src(), ip.dst());

			ip.total_length(size_guard.head_size() - ip_off);
			ip.update_checksum();

			submit_tx_packet(pd);
		}

		explicit Backend_base(Genode::Env &env,
		                      Genode::Allocator &alloc,
		                      Genode::Xml_node config,
		                      Genode::Xml_node policy)
		:
		  _link_state_handler(env.ep(), *this, &Backend_base::_handle_link_state),
		  _rx_packet_handler(env.ep(), *this, &Backend_base::_handle_rx_packet),
		  _udp_port(policy.attribute_value("udp_port", Port(9009))),
		  _timer(env),

		  _tx_block_alloc(&alloc),
		  _nic(env, &_tx_block_alloc, BUF_SIZE, BUF_SIZE),

		  _mac_address(_nic.mac_address()),
		  _dst_mac  (policy.attribute_value("dst_mac", Ethernet_frame::broadcast())),
		  _src_ip   (policy.attribute_value("src", Ipv4_packet::current())),
		  _accept_ip(policy.attribute_value("src", Ipv4_packet::broadcast())),
		  _dst_ip   (policy.attribute_value("dst", Ipv4_packet::broadcast())),
		  _chksum_offload(config.attribute_value("chksum_offload", _chksum_offload))
		{
			_nic.link_state_sigh(_link_state_handler);
			_nic.rx_channel()->sigh_packet_avail(_rx_packet_handler);
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
			/* check for acknowledgements */
			_tx_ack();

			if (!_nic.tx()->ready_to_submit()) {
				Genode::error("not ready to submit");
				return;
			}

			_nic.tx()->submit_packet(packet);
		}

		/**
		 * Handle an Ethernet packet
		 *
		 * \param src   Ethernet frame's address
		 * \param size  Ethernet frame's size.
		 */
		void handle_ethernet(void* src, Genode::size_t size);

		/*
		 * Handle an ARP packet
		 *
		 * \param eth   Ethernet frame containing the ARP packet.
		 * \param size  size guard
		 */
		void handle_arp(Ethernet_frame &eth,
		                Size_guard     &edguard);

		/*
		 * Handle an IP packet
		 *
		 * \param eth   Ethernet frame containing the IP packet.
		 * \param size  size guard
		 */
		void handle_ip(Ethernet_frame &eth,
		               Size_guard     &edguard);

		/**
		 * Send ethernet frame
		 *
		 * \param eth   ethernet frame to send.
		 * \param size  ethernet frame's size.
		 */
		void send(Ethernet_frame *eth, Genode::size_t size);
};

#endif
