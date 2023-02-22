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

#ifndef _INCLUDE__OS__RDRAND_H_
#define _INCLUDE__OS__RDRAND_H_

#include <base/stdint.h>
#include <base/log.h>
#include <base/exception.h>

namespace Genode { namespace Rdrand {

	constexpr
	bool supported() { return false; }

	uint64_t random64()
	{
		error("RDRAND not available on this architecture");
		throw Exception();
	}

} }

#endif /* _INCLUDE__OS__RDRAND_H_ */
