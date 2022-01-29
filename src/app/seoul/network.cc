/*
 * \brief  Network receive handler per MAC address
 * \author Markus Partheymueller
 * \author Alexander Boettcher
 * \date   2012-07-31
 */

/*
 * Copyright (C) 2012 Intel Corporation
 * Copyright (C) 2013-2017 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 *
 * The code is partially based on the Vancouver VMM, which is distributed
 * under the terms of the GNU General Public License version 2.
 *
 * Modifications by Intel Corporation are contributed under the terms and
 * conditions of the GNU General Public License version 2.
 */

/* local includes */
#include "network.h"


Seoul::Network::Network(Genode::Env &env, Genode::Heap &heap,
                        Synced_motherboard &mb)
:
	_motherboard(mb), _tx_block_alloc(&heap),
	_nic(env, &_tx_block_alloc, BUF_SIZE, BUF_SIZE),
	_rx_handler(env.ep(), *this, &Network::_handle_rx)
{
	_nic.rx_channel()->sigh_packet_avail(_rx_handler);
	_nic.rx_channel()->sigh_ready_to_ack(_rx_handler);
	/* vCPU EPs are synced by sync_motherboard, but adding main EP due
	 * to an own handler causes SMP trouble - avoid it
	_nic.tx_channel()->sigh_ready_to_submit(_tx_handler);
	_nic.tx_channel()->sigh_ack_avail(_tx_handler);
	*/
}


void Seoul::Network::_handle_rx()
{
	while (_nic.rx()->packet_avail()) {

		if (!_nic.rx()->ready_to_ack()) {
			Genode::warning("network: not ready for receive ack");
			break;
		}

		Nic::Packet_descriptor rx_packet = _nic.rx()->get_packet();

		/* send it to the network bus */
		char * rx_content = _nic.rx()->packet_content(rx_packet);
		_forward_pkt = rx_content;
		MessageNetwork msg((unsigned char *)rx_content, rx_packet.size(), 0);
		_motherboard()->bus_network.send(msg);
		_forward_pkt = 0;

		/* acknowledge received packet */
		_nic.rx()->acknowledge_packet(rx_packet);
	}
}


void Seoul::Network::_handle_tx()
{
	/* check for acknowledgements */
	while (_nic.tx()->ack_avail()) {
		Nic::Packet_descriptor const ack = _nic.tx()->get_acked_packet();
		_nic.tx()->release_packet(ack);
	}
}


bool Seoul::Network::transmit(void const * const packet, Genode::size_t len)
{
	if (packet == _forward_pkt)
		/* don't end in an endless forwarding loop */
		return false;

	/* check for acknowledgements */
	_handle_tx();

	/* exception is no option */
	if (!_nic.tx()->ready_to_submit()) {
		Genode::warning("network: congested - submit issue\n");
		return false;
	}

	/* allocate transmit packet */
	Nic::Packet_descriptor tx_packet;
	try {
		tx_packet = _nic.tx()->alloc_packet(len);
	} catch (Nic::Session::Tx::Source::Packet_alloc_failed) {
		Genode::warning("network: congested - alloc issue\n");
		return false;
	}

	/* fill packet with content */
	char * const tx_content = _nic.tx()->packet_content(tx_packet);
	memcpy(tx_content, packet, len);

	_nic.tx()->submit_packet(tx_packet);

	return true;
}
