/*
 * \brief  Initialize functions
 * \author Josef Soentgen
 * \date   2017-08-01
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__LWEXT4_INIT_H_
#define _INCLUDE__LWEXT4_INIT_H_

/* Genode includes */
#include <base/exception.h>

namespace Genode {
	struct Env;
	struct Allocator;
}

struct ext4_blockdev;

namespace Lwext4 {

	struct Block_init_failed  : Genode::Exception { };
	struct Malloc_init_failed : Genode::Exception { };

	void malloc_init(Genode::Env &, Genode::Allocator &heap);
	struct ext4_blockdev *block_init(Genode::Env &, Genode::Allocator &heap);
}

#endif /* _INCLUDE__LWEXT4_INIT_H_ */
