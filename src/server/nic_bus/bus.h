/*
 * \brief  Nic session bus
 * \author Emery Hemingway
 * \date   2019-04-14
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _NETWORK_STATE_H_
#define _NETWORK_STATE_H_

/* Genode includes */
#include <net/ethernet.h>
#include <base/session_label.h>
#include <util/xml_node.h>

namespace Nic_bus {
	using namespace Net;
	using namespace Genode;

	template <typename T>
	struct Bus;
}


template <typename T>
struct Nic_bus::Bus
{
		struct Element;

		/* static array of sessions on the bus */
		enum { BUS_SIZE = 0xffU };
		Element *_elements[BUS_SIZE] { nullptr };

		static uint8_t _index(Mac_address const &mac) { return mac.addr[1]; };

		void remove(Mac_address const mac) {
			_elements[_index(mac)] = nullptr; }

		Mac_address insert(Element &elem, char const *label)
		{
			/**
			 * Derive a MAC address using the FNV-1a algorithm.
			 */
			enum {
				FNV_64_PRIME = 1099511628211U,
				FNV_64_OFFSET = 14695981039346656037U,
			};

			uint64_t hash = FNV_64_OFFSET;

			char const *p = label;
			while (*p) {
				hash ^= *p++;
				hash *= FNV_64_PRIME;
			}

			for (;;) {
				/* add the terminating zero */
				hash *= FNV_64_PRIME;

				uint8_t index = hash >> 32;

				if (_elements[index] != nullptr)
					continue;
				/* hash until a free slot is found */

				_elements[index] = &elem;

				Mac_address mac;
				mac.addr[0] = 0x02;
				mac.addr[1] = index;
				mac.addr[2] = hash >> 24;
				mac.addr[3] = hash >> 16;
				mac.addr[4] = hash >> 8;
				mac.addr[5] = hash;

				return mac;
			}
		}

		struct Element
		{
			Bus &bus;
			T   &obj;

			Mac_address const mac;

			Element(Bus &b, T &o, char const *label)
			: bus(b), obj(o), mac(bus.insert(*this, label)) { }

			~Element() { bus.remove(mac); }
		};

		Bus() { }

		template<typename PROC>
		void apply(Mac_address mac, PROC proc)
		{
			uint8_t const index = _index(mac);
			Element *elem = _elements[index];
			if (elem != nullptr) {
				if (elem->mac == mac) {
					proc(elem->obj);
				}
			}
		}

		template<typename PROC>
		void apply_all(PROC proc)
		{
			for (auto i = 0U; i < BUS_SIZE; ++i) {
				Element *elem = _elements[i];
				if (elem != nullptr) {
					proc(elem->obj);
				}
			}
		}
};

#endif
