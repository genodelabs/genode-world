/*
 * \brief   Utility for reading hardware RNG
 * \date    2019-02-05
 * \author  Emery Hemingway
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__SPEC__X86_64__OS__RDRAND_H_
#define _INCLUDE__SPEC__X86_64__OS__RDRAND_H_

#include <base/stdint.h>
#include <base/log.h>
#include <base/exception.h>

namespace Genode { namespace Rdrand {

	bool supported()
	{
		unsigned int info[4] = {0, 0, 0, 0};

		asm volatile("cpuid":"=a"(info[0]),"=b"(info[1]),"=c"(info[2]),"=d"(info[3]):"a"(1),"c"(0));

		enum { RDRAND_MASK = 0x40000000 };
		return (info[2] & RDRAND_MASK);
	}

#	define _rdrand_step(x) \
	({ \
		unsigned char err; \
		asm volatile("rdrand %0; setc %1":"=r"(x), "=qm"(err)); \
		err; \
	}) \

	uint64_t random64()
	{
		uint64_t result = 0;

		enum { RETRIES = 8 };
		for (int i = 0; i < RETRIES; i++)
			if (_rdrand_step(result))
				return result;

		error("RDRAND failure");
		throw Exception();
	}

#	undef _rdrand_step

} }

#endif /* _INCLUDE__SPEC__X86_64__OS__RDRAND_H_ */
