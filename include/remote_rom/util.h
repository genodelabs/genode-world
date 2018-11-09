/*
 * \brief  Utility functions used by ROM forwarder and receiver.
 * \author Johannes Schlatow
 * \date   2018-11-14
 */

/*
 * Copyright (C) 2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef __INCLUDE__REMOTE_ROM__UTIL_H_
#define __INCLUDE__REMOTE_ROM__UTIL_H_

#include <base/fixed_stdint.h>

namespace Remote_rom {
	using Genode::uint32_t;
	using Genode::int32_t;
	using Genode::uint8_t;
	using Genode::size_t;

	uint32_t cksum(void const * const buf, size_t size);
}
/**
 * Calculating checksum compatible to POSIX cksum
 *
 * \param buf   pointer to buffer containing data
 * \param size  length of buffer in bytes
 *
 * \return CRC32 checksum of data
 */
Genode::uint32_t Remote_rom::cksum(void const * const buf, size_t size)
{
	uint8_t const *p = static_cast<uint8_t const*>(buf);
	uint32_t crc = ~0U;

	while (size--) {
		crc ^= *p++;
		for (uint32_t j = 0; j < 8; j++)
			crc = (-int32_t(crc & 1) & 0xedb88320) ^ (crc >> 1);
	}

	return crc ^ ~0U;
}

#endif
