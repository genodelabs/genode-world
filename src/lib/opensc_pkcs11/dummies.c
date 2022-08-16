/*
 * \brief  Dummies needed by opensc
 * \author Josef Soentgen
 * \date   2022-07-12
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/*
 * The functions below are not needed as there is no
 * memory swapping functionality available on Genode.
 */

#include <sys/mman.h>

/*
 * Called by sc_mem_secure_alloc.
 */

int mlock(const void *addr, size_t size)
{
	(void)addr;
	(void)size;
	return 0;
}


/*
 * Called by sc_mem_secure_free.
 */

int munlock(const void *addr, size_t size)
{
	(void)addr;
	(void)size;
	return 0;
}
