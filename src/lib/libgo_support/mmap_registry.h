/*
 * \brief  Registry for keeping track of mmapped regions
 *         Based on Genode framework
 * \author Alexander Tormasov
 * \date   2021-12-16
 *
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

#ifndef _LIBGO_SUPPORT__MMAP_REGISTRY_H_
#define _LIBGO_SUPPORT__MMAP_REGISTRY_H_

/* Genode includes */
#include <base/mutex.h>
#include <base/env.h>
#include <base/log.h>
#include <libc/allocator.h>
#include <util/list.h>


namespace Libgo_support {

	using namespace Genode;

	class Mmap_registry;

	/**
	 * Return singleton instance of mmap registry
	 */
	Mmap_registry *mmap_registry();
}


class Libgo_support::Mmap_registry
{
	public:

		struct Entry : List<Entry>::Element
		{
			void * const start;

			Entry(void *start) : start(start) { }
		};

	private:

		Libc::Allocator _md_alloc;

		List<Mmap_registry::Entry> _list;

		Mutex mutable _mutex;

		/*
		 * Common for both const and non-const lookup functions
		 */
		template <typename ENTRY>
		static ENTRY *_lookup_by_addr_unsynchronized(ENTRY *curr, void * const start)
		{
			for (; curr; curr = curr->next())
				if (curr->start == start)
					return curr;

			return 0;
		}

		Entry const *_lookup_by_addr_unsynchronized(void * const start) const
		{
			return _lookup_by_addr_unsynchronized(_list.first(), start);
		}

		Entry *_lookup_by_addr_unsynchronized(void * const start)
		{
			return _lookup_by_addr_unsynchronized(_list.first(), start);
		}

	public:

		void insert(void *start, size_t len)
		{
			Mutex::Guard guard(_mutex);

			if (_lookup_by_addr_unsynchronized(start)) {
				warning(__func__, ": anon mmap region at ", start, " "
				        "is already registered");
				return;
			}

			_list.insert(new (&_md_alloc) Entry(start));
		}

		bool registered(void *start) const
		{
			Mutex::Guard guard(_mutex);

			return _lookup_by_addr_unsynchronized(start) != 0;
		}

		void remove(void *start)
		{
			Mutex::Guard guard(_mutex);

			Entry *e = _lookup_by_addr_unsynchronized(start);

			if (!e) {
				warning("lookup for address ", start, " "
				        "in in anon mmap registry failed");
				return;
			}

			_list.remove(e);
			destroy(&_md_alloc, e);
		}
};


#endif /* _LIBGO_SUPPORT__MMAP_REGISTRY_H_ */
