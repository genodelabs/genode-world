/*
 * \brief  Implementation of mmap() with partial MAP_FIXED|MAP_ANON
 *         based on Genode jdk os_genode.cpp code
 * \author Alexander Tormasov
 * \date   2020-11-17
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

#include <base/heap.h>
#include <base/registry.h>
#include <libc/component.h>
#include <base/attached_rom_dataspace.h>
#include <region_map/client.h>
#include <rm_session/connection.h>
#include <util/retry.h>
#include <base/log.h>
#include <libc/allocator.h>

extern "C"
{
	/* libc includes */
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/cdefs.h>
#include <assert.h>
}

static const int throw_base = -100;

/**************************
 ** VM region management **
 **************************/

namespace Genode {

	class Vm_area;
	class Vm_area_registry;
	class Vm_region_map;
}

/*
 * intended for mmap with ANON memory
 *
 * reserve: for range we allocate local virtual memory (lvm) from
 *   local allocator to preserve it from usage later by lvm allocator
 *   any access to the range will lead to fault (equal to PROT_NONE)
 *
 * commit: we allocate phys mem as ds, and map it "below" range with
 *   appropriate access type (exec or read/write)
 *
 * release: could be "full" area release (symmetric to reserve and commit),
 *   or could be in the middle of ds commited
 *   in such case we will not free backend memory from ds because we can't
 *   make partial free, but just unmap memory from lvm (and force fault in
 *   case of access). So, any such unmap will make a hole in allocated ds
 *   We do not free/update local allocator if we make hole - because it is
 *   not used for handling of mapped/unmapped areas, this is solely
 *   responsibility of user created anon mapping
 *
 *   TODO: track unused memory after partial release from ds to reuse it later
 */
class Genode::Vm_region_map
{
	protected:
		Env &_env;
		Allocator_avl _range;
		addr_t _base;
		size_t _size;
		Rm_connection _rm_connection;
		Region_map_client _rm;

	public:
		typedef Region_map_client::Local_addr Local_addr;

		Vm_region_map(Env &env, Allocator &md_alloc,
		              size_t bytes = 0,
		              addr_t requested_addr = 0)
		: _env(env), _range(&md_alloc),
		  _base(requested_addr), _size(bytes),
		  _rm_connection(_env), _rm(_rm_connection.create(_size))
		{
			/* attach full chunk of dataspace as requested local virtual address */
			if (_base) {
				void *la = 0;
				la = _env.rm().attach_at(_rm.dataspace(), _base);
				if ((addr_t)la != _base) {
					Genode::error(
						"Vm_region_map::Vm_region_map: cannot attach_at ",
						Hex_range<addr_t>(_base, _size),
						" received ", Hex((addr_t)la));
					throw Region_map::Region_conflict();
				}
			} else
				_base = _env.rm().attach(_rm.dataspace());
		}

		~Vm_region_map()
		{
			/* ??? FIXME - need to cleanup _range */
			_env.rm().detach(_base);
		}

		addr_t base() const { return _base; }
		size_t size() const { return _size; }

		Range_allocator::Range_result add_range(addr_t base, size_t size) { return _range.add_range(base, size); }

		Range_allocator::Range_result remove_range(addr_t base, size_t size) { return _range.remove_range(base, size); }

		addr_t alloc(size_t size, int align)
		{
			return _range.alloc_aligned(size, align > 12 ? align : 12).convert<addr_t>(
				[&] (void *ptr) { return (addr_t)ptr; },
				[&] (Range_allocator::Alloc_error) -> addr_t {
					error(
						"Vm_region_map::alloc error: ",
						Hex(size));
					throw -1 + throw_base; });
		}

		addr_t alloc_at(size_t size, addr_t addr)
		{
			return _range.alloc_addr(size, addr).convert<addr_t>(
				[&] (void *ptr) { return (addr_t)ptr; },
				[&] (Range_allocator::Alloc_error) -> addr_t {
					error(
						"Vm_region_map::alloc_at error: ",
						Hex_range<addr_t>(addr, size));
					throw -2 + throw_base; });
		}

		void free(addr_t vaddr) { _range.free((void *)vaddr); }

		// size_t size_at(void const *addr) const { return _range.size_at(addr); }

		Local_addr attach_at(Dataspace_capability ds, addr_t local_addr, size_t size)
		{
			return retry<Genode::Out_of_ram>(
				[&] () {
					return _rm.attach_at(ds, local_addr - _base, size);
				},
				[&] () { _env.upgrade(Parent::Env::pd(), "ram_quota=8K"); });
		}

		Local_addr attach_executable(Dataspace_capability ds, addr_t local_addr, size_t size)
		{
			return retry<Genode::Out_of_ram>(
				[&] () {
					return _rm.attach_executable(ds, local_addr - _base, size);
				},
				[&] () { _env.upgrade(Parent::Env::pd(), "ram_quota=8K"); });
		}

		void detach(Local_addr local_addr) { _rm.detach((addr_t)local_addr - _base); }
};

class Genode::Vm_area
{
	private:
		struct Vm_range
		{
			addr_t base;
			size_t size;

			bool get_crossing_range(const addr_t q_base, size_t q_size,
			                        addr_t &crossing_base, size_t &crossing_size)
			{
				/* if inside modify crossing_base/crossing_size and return true */

				/* known bug - not work for q_size == 0 */
				if ((q_base + (q_size ? q_size : 1) > base) &&
				    (base + size > q_base)) {
					crossing_base = max(q_base, base);
					crossing_size = min(q_base + q_size, base + size) - crossing_base;
					return true;
				}
				return false;
			}

			Vm_range(addr_t base, size_t size)
			: base(base), size(size) {}

			virtual ~Vm_range()
			{
			}
		};

		struct Vm_area_ds : Vm_range
		{
			Ram_dataspace_capability ds;

			Vm_area_ds(addr_t base, size_t size, Ram_dataspace_capability _ds)
			: Vm_range(base, size), ds(_ds) {}

			virtual ~Vm_area_ds()
			{
			}
		};

		typedef Registered<Vm_area_ds> vm_handle;

		Env &_env;
		Allocator &_kheap;
		Vm_region_map _rm;
		Registry<vm_handle> _ds;

	public:

		typedef Registered<Vm_range> vm_ranges_handle;
		Registry<vm_ranges_handle> mapped;

		Vm_area(Env &env, Allocator &heap, addr_t base, size_t size)
		: _env(env), _kheap(heap), _rm(env, heap, size, base)
		{
		}

		addr_t base() const { return _rm.base(); }
		size_t size() const { return _rm.size(); }

		bool inside(addr_t base, size_t size)
		{
			return base >= _rm.base() && (base + size) <= (_rm.base() + _rm.size());
		}

		Range_allocator::Range_result add_range(addr_t base, size_t size) { return _rm.add_range(base, size); }

		Range_allocator::Range_result remove_range(addr_t base, size_t size) { return _rm.remove_range(base, size); }

		addr_t alloc(size_t size, int align) { return _rm.alloc(size, align); }

		addr_t alloc_at(size_t size, addr_t local_addr) { return _rm.alloc_at(size, local_addr); }

		void free(addr_t vaddr) { _rm.free(vaddr); }

		// size_t size_at(void const *addr) const { return _rm.size_at(addr); }

		bool commit(addr_t base, size_t size, bool executable)
		{
			if (!inside(base, size))
				return false;

			Ram_dataspace_capability ds = _env.ram().alloc(size);

			try
			{
				if (executable)
					_rm.attach_executable(ds, base, size);
				else
					_rm.attach_at(ds, base, size);
			}
			catch (Region_map::Region_conflict)
			{
				Genode::warning("Region_conflict in _rm.attach ",
				                Hex_range<addr_t>(base, size));
				_env.ram().free(ds);
				return false;
			}

			/* not sure that we need to catch other exceptions
			catch (...)
			{
			    Genode::error(
			        "Vm_area::commit: exception");
			    _env.ram().free(ds);
			    return false;
			} */

			new (_kheap) vm_handle(_ds, base, size, ds);

			/* registry of used adresses in the alloced ranges for list
			 * of regions backed by phys mem
			 * we have on the same Vm_area 2 lists of ranges
			 * first for ds in _ds and second in mapped for attached backend
			 * any range from mapped should be backened by any from _ds
			 * but any range in _ds could have a holes inside (not mapped)
			 * initially we map vm_handle to vm_ranges_handle 1:1, but munmap
			 * can split them
			 */
			new (_kheap) vm_ranges_handle(mapped, base, size);

			return true;
		}

		virtual ~Vm_area()
		{
			mapped.for_each([&](vm_ranges_handle &vr) {
				                /* detach from local virtual memory */
				                _rm.detach(vr.base);

				                /* remove structure/metadata from local heap allocator */
				                destroy(_kheap, &vr);
			                });
			_ds.for_each([&](vm_handle &vm) {
				             /* remove from local allocator */
				             _rm.free(vm.base);

				             /* free backend memory/dataspace (physical) */
				             _env.ram().free(vm.ds);

				             /* remove structure/metadata from local heap allocator */
				             destroy(_kheap, &vm);
			             });
		}
};

class Genode::Vm_area_registry
{
	private:
		typedef Registered<Vm_area> Vm_area_handle;

		Env &_env;
		Allocator &_kheap;
		Registry<Vm_area_handle> _registry;

		/* dedicated area to store memory allocated without desired address */
		Vm_area *_pvma;

	public :

		Vm_area_registry(Env &env, Allocator &heap, size_t size,
		                 addr_t requested_address = 0)
	: _env(env), _kheap(heap)
	{
		/* register dedicated Vm_area for first allocator range, used without requested_addr */
		_pvma = new (&_kheap) Vm_area_handle(_registry, _env, _kheap, requested_address, size);

		/* add mapped range to local allocator to allow its use in allocation */
		_pvma->add_range(_pvma->base(), size);
	}

	// ~Vm_area_registry()
	// {
	// 	_registry.for_each([&](Vm_area_handle &vma) {
	// 		                   destroy(_kheap, &vma);
	// 	                   });
	// }

	addr_t reserve(size_t size, addr_t base, int align)
	{
		if (!base) {
			/* from pre-allocated and commited _pvma area, it is already inside add_range() */
			base = _pvma->alloc(size, align);
		} else {
			/* store registered structure for this mapping */
			Vm_area *vm = new (&_kheap) Vm_area_handle(_registry, _env, _kheap, base, size);

			/* add range for requested addresses */
			vm->add_range(base, size).with_error(
				[&] (Range_allocator::Alloc_error err) {
					error(__func__, " cant reserve, err: ", err, " align ", Hex(align), " ", Hex_range<addr_t>(base, size));
					throw -3 + throw_base; });

			/* add it as a whole to preserve from local allocator usage */
			vm->alloc_at(size, base);
		}

		return base;
	}

	bool commit(addr_t base, size_t size, bool executable)
	{
		bool success = false;

		_registry.for_each([&](Vm_area_handle &vm) {
			                   if (success)
				                   return;
			                   success = vm.commit(base, size, executable);
		                   });

		return success;
	}

	addr_t get_base_address(addr_t base, bool &nanon, size_t &size)
	{
		bool success = false;
		addr_t ret = 0;

		_registry.for_each([&](Vm_area_handle &vm) {
			                   if (success)
				                   return;
			                   if (vm.inside(base, 1)) {
				                   ret = vm.base();
				                   nanon = (ret != _pvma->base());
				size = vm.size();
				return;
			}
		});

		return ret;
	}

	bool release(addr_t base, size_t size, bool &area_used)
	{
		bool success = false;

		_registry.for_each([&](Vm_area_handle &vm) {
			                   if (success || !vm.inside(base, size))
				                   return;

			                   if (base != vm.base() || size != vm.size()) {
				                   /* do split vm */
				                   bool used = false;
				                   bool need_new_chunk = false;
				                   addr_t newaddr, oldaddr;
				                   size_t newsize, oldsize;

				                   /* search for overlapping region in current Vm_area */
				                   vm.mapped.for_each([&](Vm_area::vm_ranges_handle &vr) {
					                                      if (vr.get_crossing_range(base, size, newaddr, newsize)) {
						/* found crossing range of input with our current vr
						 * ranges:
						 *   vr.base, vr.size - how chunk was allocated, we assume to be inside
						 *  [vr.base,newaddr): used (can be empty)
						 *  [newaddr,newaddr+newsize): to became free
						 *  [newaddr+newsize,vr.base+vr.size): used (can be empty)
						 */
						                                      bool used_vr = false;
						                                      oldaddr = vr.base;
						                                      oldsize = vr.size;

						                                      /* reuse first chunk if need */
						                                      if (oldaddr != newaddr) {
							                                      vr.size = newaddr - oldaddr;
							                                      used_vr = true;
						                                      }

						                                      /* create second chunk if need */
						                                      if (newaddr + newsize != oldaddr + oldsize) {
							                                      if (used_vr) {
								/* can't create and add new object inside for_each()
								 * due to recursive mutex call (which is not allowed) */
								                                      need_new_chunk = true;
								/* this is not empty vr, and no iteration required
								 * because we are in the middle of the current vr, it cant cross other */
								                                      return;
							                                      }
							                                      else {
								                                      /* reuse original vr */
								                                      vr.base = newaddr + newsize;
								                                      vr.size = oldaddr + oldsize - newaddr - newsize;
								                                      used_vr = true;
							                                      }
						                                      }

						                                      /* if not used vr - free it */
						                                      if (!used_vr) {
							                                      destroy(_kheap, &vr);
						                                      }
						                                      else
							                                      used = true;
					                                      }
					                                      else
						                                      used = true;
				                                      });

				                   if (need_new_chunk) {
					                   new (_kheap) Vm_area::vm_ranges_handle(vm.mapped, newaddr + newsize,
					                                                          oldaddr + oldsize - newaddr - newsize);
				                   }

				                   area_used = used;
				                   success = true;
				                   return;
			                   }

			                   /* whole area free */
			                   vm.free(vm.base());

			                   /* do not destroy main area for requested_address==0 allocation */
			                   if (_pvma != &vm)
				                   destroy(_kheap, &vm);
			                   success = true;
			                   area_used = false;
		                   });

		if (!success)
			error(__func__, " Vm_area_registry::release failed at ", Hex_range<addr_t>(base, size));

		return success;
	}
};

static Genode::Constructible<Genode::Vm_area_registry> vm_reg;

namespace Genode {

	char *pd_reserve_memory(size_t bytes, void *requested_addr,
	                        size_t alignment_hint);
	bool pd_unmap_memory(void *addr, size_t bytes, bool &area_used);
	bool pd_commit_memory(void *addr, size_t size, bool exec, bool with_requested_addr);
	void *pd_get_base_address(void *addr, bool &anon, size_t &size);
}

char *Genode::pd_reserve_memory(size_t bytes, void *requested_addr,
                                size_t alignment_hint)
{
	try
	{
		Genode::addr_t addr;
		addr = vm_reg->reserve(bytes, (Genode::addr_t)requested_addr,
		                       alignment_hint ? Genode::log2(alignment_hint) : 12);

		return (char *)addr;
	}
	catch (...)
	{
		/* some platform like sel4 do not allow to use 0xc000000000 for arena
		 * in this place could be waterfall of alloc exception messages

		Genode::error(__PRETTY_FUNCTION__, " alloc exception, ",
					  Hex_range((Genode::addr_t)requested_addr, bytes));
		 */
	}
	return nullptr;
}

bool Genode::pd_unmap_memory(void *addr, size_t bytes, bool &area_used)
{
	return vm_reg->release((Genode::addr_t)addr, bytes, area_used);
}

bool Genode::pd_commit_memory(void *addr, size_t size, bool exec, bool with_requested_addr)
{
	if (!addr) {
		Genode::error(__PRETTY_FUNCTION__, "  addr == 0");
		throw -7 + throw_base;
	}

	return vm_reg->commit((Genode::addr_t)addr, size, exec);
}

void *Genode::pd_get_base_address(void *addr, bool &nanon, size_t &size)
{
	return (void *)vm_reg->get_base_address((addr_t)addr, nanon, size);
}

/******************
 ** Startup code **
 ******************/

namespace Libc {

	void anon_mmap_construct(Genode::Env &env, size_t default_size);
}

void Libc::anon_mmap_construct(Genode::Env &env, size_t default_size)
{
	static Libc::Allocator libc_alloc { };

	/* get Kernel::_heap from Libc */
	/* and use it as metadata data storage with pre-allocaion */
	vm_reg.construct(env, libc_alloc, default_size);
}
