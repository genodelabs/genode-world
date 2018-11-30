/*
 * \brief Client implementation
 * \author Johannes Schlatow
 * \author Edgard Schmidt
 * \date   2018-11-06
 */

#include <base.h>

void Remote_rom::Backend_base::_handle_rx_packet()
{
	/* we assume that the SUBMIT and ACK queue are equally dimensioned
	 * i.e. ready_to_ack should always return true */
	while (_nic.rx()->packet_avail() && _nic.rx()->ready_to_ack()) {
		Packet_descriptor pd = _nic.rx()->get_packet();
		if (pd.size())
			handle_ethernet(_nic.rx()->packet_content(pd), pd.size());

		_nic.rx()->acknowledge_packet(pd);
	}
}

void Remote_rom::Backend_base::_handle_arp_request(Ethernet_frame &eth,
                                                   Size_guard     &guard,
                                                   Arp_packet     &arp)
{
	/*
	 * The ARP packet gets re-written, we interchange source
	 * and destination MAC and IP addresses, and set the opcode
	 * to reply, and then push the packet back to the NIC driver.
	 */
	Ipv4_address old_src_ip = arp.src_ip();
	arp.opcode(Arp_packet::REPLY);
	arp.dst_mac(arp.src_mac());
	arp.src_mac(_mac_address);
	arp.src_ip(arp.dst_ip());
	arp.dst_ip(old_src_ip);
	eth.dst(arp.dst_mac());

	/* set our MAC as sender */
	eth.src(_mac_address);
	send(&eth, guard.total_size());
}

void Remote_rom::Backend_base::handle_ethernet(void* src, Genode::size_t size)
{
	try {
		/* parse ethernet frame header */
		Size_guard edguard(size);
		Ethernet_frame &eth = Ethernet_frame::cast_from(src, edguard);
		switch (eth.type()) {
		case Ethernet_frame::Type::ARP:
			handle_arp(eth, edguard);
			break;
		case Ethernet_frame::Type::IPV4:
			handle_ip(eth, edguard);
			break;
		default:
			;
		}
	} catch(Size_guard::Exceeded) {
		Genode::warning("Packet size guard exceeded!");
	}
}

void Remote_rom::Backend_base::handle_arp(Ethernet_frame &eth,
                                          Size_guard     &edguard)
{
	/* ignore broken packets */
	Arp_packet &arp = eth.data<Arp_packet>(edguard);
	if (!arp.ethernet_ipv4())
		return;

	/* look whether the IP address is our's */
	if (arp.dst_ip() == _accept_ip) {
		switch (arp.opcode()) {
			case Arp_packet::REQUEST:
				_handle_arp_request(eth, edguard, arp);
				break;
			case Arp_packet::REPLY:
				if (_arp_waiting && arp.src_ip() == _dst_ip) {
					_dst_mac = arp.src_mac();
					_arp_waiting = false;
				}
				break;
			default: break;
		}
	}
}

void Remote_rom::Backend_base::handle_ip(Ethernet_frame &eth,
                                         Size_guard     &edguard)
{
	Ipv4_packet &ip = eth.data<Ipv4_packet>(edguard);
	if (_accept_ip == Ipv4_packet::broadcast()
		 || _accept_ip == ip.dst()) {

		if (ip.protocol() == Ipv4_packet::Protocol::UDP) {
			if (_dst_mac == Ethernet_frame::broadcast() && ip.src() == _dst_ip) {
				_dst_mac = eth.src();
			}
			/* remote_rom protocol is wrapped in UDP */
			Udp_packet &udp = ip.data<Udp_packet>(edguard);
			if (udp.dst_port() == _udp_port)
				receive(udp.data<Packet>(edguard), edguard);
		}
	}
}

void Remote_rom::Backend_base::send(Ethernet_frame *eth, Genode::size_t size)
{
	try {
		/* copy and submit packet */
		Packet_descriptor packet  = _nic.tx()->alloc_packet(size);
		char             *content = _nic.tx()->packet_content(packet);
		Genode::memcpy((void*)content, (void*)eth, size);
		submit_tx_packet(packet);

	} catch(Nic::Session::Tx::Source::Packet_alloc_failed) {
		Genode::warning("Packet dropped");
	}
}
