/*
 * \brief  Common base for client and server
 * \author Johannes Schlatow
 * \date   2016-02-18
 */

#include <base/env.h>
#include <base/log.h>
#include <base/attached_rom_dataspace.h>

#include <nic/packet_allocator.h>
#include <nic_session/connection.h>

#include <net/ethernet.h>
#include <net/ipv4.h>

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

	class  Backend_base;
};


class Remote_rom::Backend_base : public Genode::Interface
{
	private:

		Genode::Signal_handler<Backend_base> _link_state_handler;
		Genode::Signal_handler<Backend_base> _rx_packet_handler;

		void _handle_rx_packet()
		{
			while (_nic.rx()->packet_avail() && _nic.rx()->ready_to_ack()) {
				Packet_descriptor _rx_packet = _nic.rx()->get_packet();

				char *content = _nic.rx()->packet_content(_rx_packet);
				Size_guard edguard(_rx_packet.size());
				Ethernet_frame &eth = Ethernet_frame::cast_from(content, edguard);

				/* check IP */
				Ipv4_packet &ip_packet = eth.data<Ipv4_packet>(edguard);
				if (_accept_ip == Ipv4_packet::broadcast()
				    || _accept_ip == ip_packet.dst())
					receive(ip_packet.data<Packet>(edguard), edguard);

				_nic.rx()->acknowledge_packet(_rx_packet);
			}
		}

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
			PACKET_SIZE = 1024,
			BUF_SIZE = Nic::Session::QUEUE_SIZE * PACKET_SIZE
		};

		const bool            _verbose = false;
		Nic::Packet_allocator _tx_block_alloc;
		Nic::Connection       _nic;
		Mac_address           _mac_address;
		Ipv4_address          _src_ip;
		Ipv4_address          _accept_ip;
		Ipv4_address          _dst_ip;

		/**
		 * Handle accepted network packet from the other side
		 */
		virtual void receive(Packet &packet, Size_guard &) = 0;

		Ipv4_packet &_prepare_upper_layers(void *base, Size_guard &size_guard)
		{
			Ethernet_frame &eth = Ethernet_frame::construct_at(base, size_guard);
			eth.src(_mac_address);
			eth.dst(Ethernet_frame::broadcast());
			eth.type(Ethernet_frame::Type::IPV4);

			Ipv4_packet &ip = eth.construct_at_data<Ipv4_packet>(size_guard);
			ip.version(4);
			ip.header_length(5);
			ip.time_to_live(10);
			ip.src(_src_ip);
			ip.dst(_dst_ip);

			return ip;
		}

		size_t _upper_layer_size(size_t size)
		{
			return sizeof(Ethernet_frame) + sizeof(Ipv4_packet) + size;
		}

		void _finish_ipv4(Ipv4_packet &ip, size_t payload)
		{
			ip.total_length(sizeof(ip) + payload);
			ip.update_checksum();
		}

		template <typename T>
		void _transmit_notification_packet(Packet::Type type, T *frontend)
		{
			size_t frame_size = _upper_layer_size(sizeof(Packet));
			Nic::Packet_descriptor pd = alloc_tx_packet(frame_size);
			Size_guard size_guard(pd.size());

			char *content = _nic.tx()->packet_content(pd);
			Ipv4_packet &ip = _prepare_upper_layers(content, size_guard);
			Packet &pak = ip.construct_at_data<Packet>(size_guard);
			pak.type(type);
			pak.module_name(frontend->module_name());
			_finish_ipv4(ip, sizeof(Packet));

			submit_tx_packet(pd);
		}

		explicit Backend_base(Genode::Env &env, Genode::Allocator &alloc)
		:
		  _link_state_handler(env.ep(), *this, &Backend_base::_handle_link_state),
		  _rx_packet_handler(env.ep(), *this, &Backend_base::_handle_rx_packet),

		  _tx_block_alloc(&alloc),
		  _nic(env, &_tx_block_alloc, BUF_SIZE, BUF_SIZE),

		  _mac_address(_nic.mac_address()),
		  _src_ip(Ipv4_packet::current()),
		  _accept_ip(Ipv4_packet::broadcast()),
		  _dst_ip(Ipv4_packet::broadcast())
		{
			_nic.link_state_sigh(_link_state_handler);
			_nic.rx_channel()->sigh_packet_avail(_rx_packet_handler);
			_nic.rx_channel()->sigh_ready_to_ack(_rx_packet_handler);

			Genode::Attached_rom_dataspace config = {env, "config"};
			try {
				char ip_string[15];
				Genode::Xml_node remoterom = config.xml().sub_node("remote_rom");
				remoterom.attribute("src").value(ip_string, sizeof(ip_string));
				_src_ip = Ipv4_packet::ip_from_string(ip_string);

				remoterom.attribute("dst").value(ip_string, sizeof(ip_string));
				_dst_ip = Ipv4_packet::ip_from_string(ip_string);

				_accept_ip = _src_ip;
			} catch (...) {
				Genode::warning("No IP configured, falling back to broadcast mode!");
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

#endif
