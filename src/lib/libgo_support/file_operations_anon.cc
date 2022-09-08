/*
 * \brief  libc file operations for anon mmap
 *         partially derived from Genode framework
 * \author Alexander Tormasov
 * \date   2010-01-21
 */

/*
 * Copyright (C) 2022 Alexander Tormasov
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * ``Software''), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* Genode includes */
#include <base/env.h>
#include <os/path.h>
#include <util/token.h>

/* local includes */
#include "mmap_registry.h"

extern "C" {
/* libc includes */
#include <errno.h>
#include <sys/mman.h>
}


using namespace Genode;

namespace Libc {

	void anon_mmap_construct(Genode::Env &env, size_t default_size);
	void anon_init_file_operations(Genode::Env &env,
	                               Xml_node const &config_accessor);
}


static Libgo_support::Mmap_registry _anon_mmap_registry;


enum { PAGE_SHIFT = 12u, };
static unsigned int  _mmap_align_log2 { PAGE_SHIFT };


void Libc::anon_init_file_operations(Genode::Env &env,
                                     Xml_node const &config_accessor)
{
	/* by default 15 Mb for anon mmap allocator without predefined address */
	enum { DEFAULT_SIZE = 15ul * 1024 * 1024 };
	size_t default_size = DEFAULT_SIZE;
	config_accessor.with_optional_sub_node("mmap", [&] (Xml_node mmap) {
			_mmap_align_log2 = mmap.attribute_value("align_log2",
			                                        _mmap_align_log2);
			default_size = mmap.attribute_value("local_area_default_size",
			                                    default_size);
	});

	anon_mmap_construct(env, default_size);
}


/***************
 ** Utilities **
 ***************/

namespace Genode {

	char *pd_reserve_memory(size_t bytes, void *requested_addr,
	                        size_t alignment_hint);
	bool pd_unmap_memory(void *addr, size_t bytes, bool &area_used);
	bool pd_commit_memory(void *addr, size_t size, bool exec, bool with_requested_addr);
	void *pd_get_base_address(void *addr, bool &anon, size_t &size);
}


extern "C" void *anon_mmap(void *addr, ::size_t length, int prot, int flags,
                           int libc_fd, ::off_t offset)
{
	/* fallback for all other mmap operations */
	bool const anon = (flags & MAP_ANONYMOUS) || (flags & MAP_ANON);
	if (!anon)
		return mmap(addr, length, prot, flags, libc_fd, offset);

	/* handle requests only for anonymous memory */
	void *start = addr;

	/* FIXME do not allow overlap with other areas as in original mmap() - just fail */

	if (prot == PROT_NONE || !addr)
	{
		/* process request for memory range reservation (no access, no commit) */
		/* desired address given as addr (mandatory if flags has MAP_FIXED) */
		start = Genode::pd_reserve_memory(length, addr, _mmap_align_log2);
		if (!start || ((flags & MAP_FIXED) && (start != addr)))
		{
			errno = ENOMEM;
			return MAP_FAILED;
		}

		_anon_mmap_registry.insert(start, length);

		/* if this is just reservation, return (no commit) */
		if (prot == PROT_NONE)
			return start;
	}

	bool const executable = prot & PROT_EXEC;

	/* desired address returned; commit virtual range */
	if ( Genode::pd_commit_memory(start, length, executable, addr != 0) )
	{
		/* zero commited ram */
		::memset(start, 0, align_addr(length, PAGE_SHIFT));
		return start;
	}

	return NULL;
}


extern "C" int anon_munmap(void *base, ::size_t length)
{
	bool nanon;
	size_t size;
	void *start = Genode::pd_get_base_address(base, nanon, size);
	if (!start)
		start = base;

	if (nanon && !_anon_mmap_registry.registered(start)) {
		return munmap(base, length);
	}

	_anon_mmap_registry.remove(start);

	bool area_used = false;
	Genode::pd_unmap_memory(base, length, area_used);
	/* if we should not remove registry - reinsert it;
	 * this could happens if we split internal area;
	 * size should be original, not length of current area
	 */
	if (!(nanon && !area_used)) {
		_anon_mmap_registry.insert(start, size);
	}
	return 0;
}


extern "C" int anon_msync(void *start, ::size_t len, int flags)
{
	if (!_anon_mmap_registry.registered(start)) {
		return msync(start, len, flags);
	}

	warning(__func__, " not implemented");
	return -1;
}
