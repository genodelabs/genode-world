/*
 * \brief  Network receive handler per MAC address
 * \author Markus Partheymueller
 * \author Alexander Boettcher
 * \date   2012-07-31
 */

/*
 * Copyright (C) 2012 Intel Corporation
 * Copyright (C) 2013-2024 Genode Labs GmbH
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

#include <service/net.h>


unsigned NicID::id;


Seoul::Network::Network(Genode::Env &env, Genode::Heap &heap,
                        Motherboard &mb, MessageHostOp &msg)
:
	_motherboard(mb), _tx_block_alloc(&heap),
	_nic(env, &_tx_block_alloc, BUF_SIZE, BUF_SIZE),
	_client_id(unsigned(msg.value)),
	_rx_handler(env.ep(), *this, &Network::_handle_rx),
	_link_state(env.ep(), *this, &Network::_handle_link)
{
	_nic.rx_channel()->sigh_packet_avail(_rx_handler);
	_nic.rx_channel()->sigh_ready_to_ack(_rx_handler);
	_nic.link_state_sigh(_link_state);

	/*
	   makes solely sense if we can stall packets in network models and resume
	   later, which is not supported by now

	_nic.tx_channel()->sigh_ready_to_submit(_tx_handler);
	_nic.tx_channel()->sigh_ack_avail(_tx_handler);
	*/

	_motherboard.bus_network.add (this, receive_static<MessageNetwork>);

	auto const mac = _nic.mac_address();

	Genode::log("network adapter ", _client_id, ", mac ", mac);

	Genode::uint64_t mac_raw = 0;
	memcpy(&mac_raw, mac.addr, 6);
	msg.mac = __builtin_bswap64(mac_raw) >> 16;

	_handle_link();
}


void Seoul::Network::_handle_rx()
{
	Genode::Mutex::Guard guard(_mutex);

	auto &rx = *_nic.rx();

	if (!rx.packet_avail())
		return;

	while (rx.packet_avail()) {

		if (!rx.ready_to_ack()) {
			Genode::warning("network: not ready for receive ack");
			break;
		}

		Nic::Packet_descriptor rx_packet = rx.try_get_packet();

		if (!rx_packet.size())
			break;

		/* send it to the network bus */
		char * rx_content = rx.packet_content(rx_packet);

		_mutex.release();

		MessageNetwork msg(MessageNetwork::PACKET_TO_MODEL,
		                   { .buffer = (unsigned char *)rx_content, .len = rx_packet.size() },
		                   _client_id, false /* no more packets */);
		_motherboard.bus_network.send(msg);

		_mutex.acquire();

		/* acknowledge received packet */
		rx.try_ack_packet(rx_packet);
	}

	rx.wakeup();
}


bool Seoul::Network::receive(MessageNetwork &msg)
{
	if (msg.type != MessageNetwork::PACKET_TO_HOST || msg.client != _client_id)
		return false;

	Genode::Mutex::Guard guard(_mutex);

	auto &tx = *_nic.tx();

	/* check for acknowledgements */
	while (tx.ack_avail())
		tx.release_packet(tx.try_get_acked_packet());

	if (!tx.ready_to_submit()) {
		Genode::warning("network: congested - submit issue - drop packet\n");
		tx.wakeup(); /* in case msg.more was set in previous invocations */
		return true;
	}

	tx.alloc_packet_attempt(msg.data.len).with_result([&] (auto &tx_packet) {
		memcpy(tx.packet_content(tx_packet), msg.data.buffer, msg.data.len);
		tx.try_submit_packet(tx_packet);
	}, [&] (auto) {
		Genode::warning("network: congested - alloc issue - drop packet\n");
	});

	if (!msg.more)
		tx.wakeup();

	return true;
}


void Seoul::Network::_handle_link()
{
	MessageNetwork msg(MessageNetwork::LINK,
	                   { .link_up = _nic.link_state() },
	                   _client_id, false);

	_motherboard.bus_network.send(msg);
}
