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

#ifndef _NETWORK_H_
#define _NETWORK_H_

/* base includes */
#include <base/heap.h>

/* os includes */
#include <nic_session/connection.h>
#include <nic/packet_allocator.h>

#include <nul/motherboard.h>


namespace Seoul {
	class Network;

	using Genode::Signal_handler;
}

class Seoul::Network : public StaticReceiver<Network>
{
	private:

		enum {
			PACKET_SIZE = Nic::Packet_allocator::DEFAULT_PACKET_SIZE,
			BUF_SIZE    = Nic::Session::QUEUE_SIZE * PACKET_SIZE,
		};

		Genode::Mutex                   _mutex { };
		Motherboard                   & _motherboard;
		Nic::Packet_allocator           _tx_block_alloc;
		Nic::Connection                 _nic;
		unsigned                const   _client_id;
		Signal_handler<Network> const   _rx_handler;
		Signal_handler<Network> const   _link_state;

		void _handle_rx();
		void _handle_link();

		Network             (Network const &);
		Network &operator = (Network const &);

	public:

		Network(Genode::Env &, Genode::Heap &, Motherboard &, MessageHostOp &);

		bool receive(MessageNetwork &msg);
};

#endif /* _NETWORK_H_ */
