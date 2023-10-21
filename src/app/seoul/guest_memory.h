/*
 * \brief  Seoul Guest memory management
 * \author Alexander Boettcher
 * \author Norman Feske
 * \author Markus Partheymueller
 * \date   2011-11-18
 */

/*
 * Copyright (C) 2011-2019 Genode Labs GmbH
 * Copyright (C) 2012 Intel Corporation
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 *
 * The code is partially based on the Vancouver VMM, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _GUEST_MEMORY_H_
#define _GUEST_MEMORY_H_

#include <base/sleep.h>
#include <rm_session/connection.h>
#include <vm_session/connection.h>
#include <region_map/client.h>

namespace Seoul {

	using namespace Genode;

	class Guest_memory;
}

class Seoul::Guest_memory
{
	private:

		Env                  &_env;
		Vm_connection        &_vm_con;
		Rm_connection         _rm_reserve   { _env };

		uint64_t       const  _guest_size;
		size_t         const  _io_mem_size  { 1ul << 30 };
		addr_t                _io_mem_alloc { 3 * (1ul << 30) }; /* configurable ? */
		addr_t                _local_addr   { };
		bool                  _io_mem_gap   { false };
		bool           const  _verbose;

		Region_map::Local_addr _reserve_local_range()
		{
			auto const ds = _rm_reserve.create(size_t(_guest_size + (_io_mem_gap ? _io_mem_size : 0)));

			Region_map_client rm(ds);
			auto const backing_store = (addr_t)_env.rm().attach(rm.dataspace());
			_rm_reserve.destroy(ds);

			/* reserve gap not to be used by dynamic allocations */
			if (_io_mem_gap) {
				auto const ds = _rm_reserve.create(_io_mem_size);

				Region_map_client rm(ds);
				auto const check = (addr_t)_env.rm().attach_at(rm.dataspace(),
				                                               backing_store + _io_mem_alloc);

				if (check != backing_store + _io_mem_alloc)
					Logging::panic("reserved range attachment failed");
			}

			return backing_store;
		}

		/*
		 * Noncopyable
		 */
		Guest_memory(Guest_memory const &);
		Guest_memory &operator = (Guest_memory const &);

		struct Region : Genode::List<Region>::Element {
			Genode::addr_t                _guest_addr;
			Genode::addr_t                _local_addr;
			Genode::Dataspace_capability  _ds;
			Genode::addr_t                _ds_size;

			Region (Genode::addr_t const guest_addr,
			        Genode::addr_t const local_addr,
			        Genode::Dataspace_capability ds,
			        Genode::addr_t const ds_size)
			: _guest_addr(guest_addr), _local_addr(local_addr),
			  _ds(ds), _ds_size(ds_size)
			{ }

			bool overlap(addr_t const addr, addr_t const size) const
			{
				if (!size)
					return true;

				if (addr < _guest_addr && addr + size - 1 < _guest_addr)
					return false;
				if (addr > _guest_addr + _ds_size - 1)
					return false;

				return true;
			}
		};

		Genode::List<Region> _regions { };

		template <typename F>
		void for_each_region(F const &fn)
		{
			for (Region *r = _regions.first(); r; r = r->next())
				fn(*r);
		}

	public:

		/**
		 * Constructor
		 *
		 * \param guest_size  number of bytes of physical RAM to be
		 *                    used as guest-physical memory,
		 *                    allocated from core's RAM service
		 */
		Guest_memory(Env &env, Allocator &alloc, Vm_connection &vm_con,
		             addr_t const guest_size, bool const verbose)
		:
			_env(env), _vm_con(vm_con), _guest_size(guest_size),
			_verbose(verbose)
		{
			auto const pg_1g = 1ul << 30;
			auto const pg_4m = 1ul << 22;

			auto max_offset = guest_size;

			for (auto offset = 0ul, ds_size = (guest_size > pg_1g) ? pg_1g : guest_size;
			     offset < max_offset;)
			{
				/* cut out io_mem region from normal memory */
				if (offset < _io_mem_alloc + _io_mem_size) {
					if (offset >= _io_mem_alloc) {
						offset = _io_mem_alloc + _io_mem_size;

						_io_mem_gap = true;
						max_offset += _io_mem_size;
						continue;
					}

					if (offset + ds_size > _io_mem_alloc) {
						ds_size = _io_mem_alloc - offset;
					}
				}

				try {
					auto const ds = env.ram().alloc(ds_size);

					/* register ds for VM region */
					bool ok = add_region(alloc, offset, 0,
					                     ds, ds_size);
					if (!ok)
						Logging::panic("guest memory allocation failed");

					offset += ds_size;

					ds_size = max_offset - offset;
					ds_size = ds_size > pg_1g ? pg_1g : ds_size;

				} catch (Genode::Ram_allocator::Denied) {

					if (_verbose)
						log("reduce ds_size ", Hex(ds_size), "->",
						    Hex(ds_size >> 1));

					ds_size = ds_size >> 1;

					if      (ds_size > pg_1g) ds_size &= ~(pg_1g - 1);
					else if (ds_size > pg_4m) ds_size &= ~(pg_4m - 1);

					if (ds_size < 4096)
						throw;

					continue;
				}
			}

			/* reserve late, due to add_region using 'new (alloc)' above */
			_local_addr = _reserve_local_range();

			for_each_region([&](auto &region) {

				region._local_addr = _local_addr + region._guest_addr;
				env.rm().attach(region._ds, 0 /* size */, 0 /* offset */,
				                true /* use local addr */,
				                region._local_addr,
				                true /* writeable */);
			});
		}

		/**
		 * Return pointer to locally mapped backing store
		 */
		char *backing_store_local_base()
		{
			return reinterpret_cast<char *>(_local_addr);
		}

		size_t backing_store_size() const
		{
			return size_t(_guest_size + (_io_mem_gap ? _io_mem_size : 0));
		}

		bool add_region(Allocator    &alloc,
		                addr_t const  guest_addr,
		                addr_t const  local_addr,
		                Dataspace_capability ds,
		                Genode::addr_t const ds_size)
		{
			if (!ds_size)
				return false;

			for_each_region([&](auto &region) {
				if (!region.overlap(guest_addr, ds_size))
					return;

				if (_verbose)
					warning("overlapping region added: ",
					        Hex(guest_addr), "+", Hex(ds_size),
					        " conflicts with ",
					        Hex(region._guest_addr), "+", Hex(region._ds_size));

				if (region._guest_addr != 0)
					Genode::sleep_forever();
			});

			if (_verbose)
				log("guest_memory: add_region ", Hex(guest_addr), "+", Hex(ds_size));

			_regions.insert(new (alloc) Region(guest_addr, local_addr, ds, ds_size));

			return true;
		}

		void dump_regions()
		{
			for_each_region([&](auto &region) {

				log("- vmm: ", Hex_range(region._local_addr, region._ds_size),
				    " - vm: ", Hex_range(region._guest_addr, region._ds_size),
				    " - ", Number_of_bytes(region._guest_addr), "+",
				           Number_of_bytes(region._ds_size));
			});
		}

		void attach_to_vm(Vm_connection &vm_con, addr_t g_phys, addr_t size,
		                  bool const writeable)
		{
			bool partial_match = false;

			do {
				partial_match = false;

				for_each_region([&](auto &region) {
					if (!region._ds_size || !size || size & 0xfffu) return;
					if (g_phys < region._guest_addr) return;
					if (g_phys > region._guest_addr + region._ds_size - 1) return;

					auto const ds_offset   = g_phys - region._guest_addr;
					auto const attach_size = min(size, region._ds_size - ds_offset);

					if (_verbose)
						log(__func__, " try attach ", Hex_range(g_phys, size),
						    " -> ", Hex_range(region._guest_addr + ds_offset, attach_size), " ",
						    " of region=", Hex_range(region._guest_addr, region._ds_size));

					vm_con.attach(region._ds, g_phys, { .offset     = ds_offset,
					                                    .size       = attach_size,
					                                    .executable = true,
					                                    .writeable  = writeable });

					if (_verbose)
						log(__func__, "   attached ", Hex_range(g_phys, attach_size),
						    " of region=", Hex_range(region._guest_addr, region._ds_size));

					size   -= attach_size;
					g_phys += attach_size;

					partial_match = true;
				});
			} while(partial_match);

			if (size)
				warning(__func__, " region not found ", Hex(g_phys), "+", Hex(size));
		}

		void detach(Genode::addr_t const guest_addr, Genode::addr_t const size)
		{
			_vm_con.detach(guest_addr, size);
		}

		Genode::addr_t alloc_io_memory(Genode::addr_t const size)
		{
			addr_t const io_mem = _io_mem_alloc;

			_io_mem_alloc += size;

			return io_mem;
		}
};

#endif /* _GUEST_MEMORY_H_ */
