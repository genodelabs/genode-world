/*
 * \brief  Quick and dirty ring buffer
 * \author Josef Soentgen
 * \date   2015-11-19
 */

/*
 * Copyright (C) 2015 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#include <base/stdint.h>
#include <util/string.h>

namespace Util {
	template <Genode::size_t> struct Ring_buffer;
}

template <Genode::size_t CAPACITY>
struct Util::Ring_buffer
{
	Genode::size_t wpos { 0 };
	Genode::size_t rpos { 0 };

	char _data[CAPACITY];

	Ring_buffer() { }

	Genode::size_t read_avail() const
	{
		if (wpos > rpos) return wpos - rpos;
		else             return (wpos - rpos + CAPACITY) % CAPACITY;
	}

	Genode::size_t write_avail() const
	{
		if      (wpos > rpos) return ((rpos - wpos + CAPACITY) % CAPACITY) - 2;
		else if (wpos < rpos) return rpos - wpos;
		else                  return CAPACITY - 2;
	}

	Genode::size_t write(void const *src, Genode::size_t len)
	{
		Genode::size_t const avail = write_avail();
		if (avail == 0) return 0;

		Genode::size_t const limit_len = len > avail ? avail : len;
		Genode::size_t const total = wpos + len;
		Genode::size_t first, rest;

		if (total > CAPACITY) {
			first = CAPACITY - wpos;
			rest  = total % CAPACITY;
		} else {
			first = limit_len;
			rest  = 0;
		}

		Genode::memcpy(&_data[wpos], src, first);
		wpos = (wpos + first) % CAPACITY;

		if (rest) {
			Genode::memcpy(&_data[wpos], ((char const*)src) + first, rest);
			wpos = (wpos + rest) % CAPACITY;
		}

		return limit_len;
	}

	Genode::size_t read(void *dst, Genode::size_t len)
	{
		Genode::size_t const avail = read_avail();
		if (avail == 0) return 0;

		Genode::size_t const limit_len = len > avail ? avail : len;
		Genode::size_t const total = rpos + len;
		Genode::size_t first, rest;

		if (total > CAPACITY) {
			first = CAPACITY - rpos;
			rest  = total % CAPACITY;
		} else {
			first = limit_len;
			rest  = 0;
		}

		Genode::memcpy(dst, &_data[rpos], first);
		rpos = (rpos + first) % CAPACITY;

		if (rest) {
			Genode::memcpy(((char*)dst) + first, &_data[rpos], rest);
			rpos = (rpos + rest) % CAPACITY;
		}

		return limit_len;
	}
};

#endif /* _RING_BUFFER_H_ */
