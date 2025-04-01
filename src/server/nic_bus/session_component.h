/*
 * \brief  Nic bus session component
 * \author Emery Hemingway
 * \date   2019-04-14
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SESSION_COMPONENT_H_
#define _SESSION_COMPONENT_H_

/* local includes */
#include "bus.h"

/* Genode includes */
#include <net/ethernet.h>
#include <net/size_guard.h>
#include <nic/component.h>
#include <base/heap.h>
#include <base/session_label.h>

namespace Nic_bus {
	using namespace Net;

	struct Tx_size { Genode::size_t value; };
	struct Rx_size { Genode::size_t value; };

	class Session_resources;
	class Session_component;

	typedef Bus<Session_component> Session_bus;
}


/**
 * Base class to manage session quotas and allocations
 */
class Nic_bus::Session_resources
{
	protected:

		Genode::Ram_quota_guard           _ram_guard;
		Genode::Cap_quota_guard           _cap_guard;
		Genode::Accounted_ram_allocator   _ram_alloc;
		Genode::Attached_ram_dataspace    _tx_ds, _rx_ds;
		Genode::Heap                      _alloc;
		Nic::Packet_allocator             _rx_pkt_alloc { &_alloc };

		Session_resources(Genode::Ram_allocator &ram,
		                  Genode::Region_map    &region_map,
		                  Genode::Ram_quota     ram_quota,
		                  Genode::Cap_quota     cap_quota,
		                  Tx_size         tx_size,
		                  Rx_size         rx_size)
		:
			_ram_guard(ram_quota), _cap_guard(cap_quota),
			_ram_alloc(ram, _ram_guard, _cap_guard),
			_tx_ds(_ram_alloc, region_map, tx_size.value),
			_rx_ds(_ram_alloc, region_map, rx_size.value),
			_alloc(_ram_alloc, region_map)
		{ }
};


class Nic_bus::Session_component : private Session_resources,
                                      public Nic::Session_rpc_object
{
	private:

		Session_bus::Element _bus_elem;

		Genode::Session_label const _label;

		Genode::Io_signal_handler<Session_component> _packet_handler;

		Nic::Packet_stream_sink<::Nic::Session::Policy> &sink() {
			return *_tx.sink(); }

		Nic::Packet_stream_source<::Nic::Session::Policy> &source() {
			return *_rx.source(); }

		void _send(Ethernet_frame const &eth, Genode::size_t const size)
		{
			while (source().ack_avail())
				source().release_packet(source().get_acked_packet());

			if (!source().ready_to_submit()) return;
			/* drop the packet if the queue is congested */

			Nic::Packet_descriptor pkt = source().alloc_packet(size);
			void *content = source().packet_content(pkt);
			Genode::memcpy(content, (void*)&eth, size);
			source().submit_packet(pkt);
		}

		void _handle_packet(Nic::Packet_descriptor const &pkt)
		{
			if (!pkt.size() || !sink().packet_valid(pkt)) return;

			Size_guard size_guard(pkt.size());
			Ethernet_frame const &eth = Ethernet_frame::cast_from(
				sink().packet_content(pkt), size_guard);

			if (eth.src() != _bus_elem.mac) {
				Genode::warning(
					eth.src(), " is not the managed MAC adress, "
					"dropping packet from ", _label);
				return;
			}

			auto send = [&] (Session_component &other) { other._send(eth, pkt.size()); };

			if (eth.dst().addr[0] & 1) {
				/* multicast */
				_bus_elem.bus.apply_all(send);
			} else {
				/* unicast */
				_bus_elem.bus.apply(eth.dst(), send);
			}
		}

		void _handle_packets()
		{
			while (sink().ready_to_ack() && sink().packet_avail()) {
				_handle_packet(sink().peek_packet());
				sink().acknowledge_packet(sink().get_packet());
			}
		}

	public:

		Session_component(Genode::Entrypoint    &ep,
		                  Genode::Ram_allocator &ram,
		                  Genode::Region_map    &region_map,
		                  Genode::Ram_quota      ram_quota,
		                  Genode::Cap_quota      cap_quota,
		                  Tx_size                tx_size,
		                  Rx_size                rx_size,
		                  Session_bus           &bus,
		                  Genode::Session_label const &label)
		:
			Session_resources(ram, region_map,
			                  ram_quota, cap_quota,
			                  tx_size, rx_size),
			Nic::Session_rpc_object(region_map,
			                        _tx_ds.cap(), _rx_ds.cap(),
			                        &_rx_pkt_alloc, ep.rpc_ep()),
			_bus_elem(bus, *this, label.string()), _label(label),
			_packet_handler(ep, *this, &Session_component::_handle_packets)
		{
			_tx.sigh_packet_avail(_packet_handler);
			_tx.sigh_ready_to_ack(_packet_handler);
		}

		Nic::Mac_address mac_address() override { return _bus_elem.mac; }

		bool link_state() override { return true; }

		void link_state_sigh(Genode::Signal_context_capability) override { }

};

#endif /* _SESSION_COMPONENT_H_ */
