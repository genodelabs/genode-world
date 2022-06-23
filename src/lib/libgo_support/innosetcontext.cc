/*
 * \brief  platform-independent setcontext wrapper to copy Thread fields for golang runtime
 * \author Alexander Tormasov
 * \date   2021-12-17
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

#include <base/thread.h>
#include <sys/ucontext.h>

using namespace Genode;

/* base-internal includes */
#include <base/internal/stack_area.h>

/* to be executed on new stack after setcontext */
extern "C"
void othercontext(void **pmyself)
{
	Thread *myself = Thread::myself();
	*pmyself = myself;
}

extern "C" long getstackaddress(ucontext_t *uc);
extern "C" void getThreadRef(ucontext_t *uc, void *pnew_myself);

/*
 * before actual call of setcontext copy over Stack and utcb/native_thread objects
 */
extern "C" int innosetcontext(ucontext_t *uc)
{
	/*
	 * current OS thread should be non-main
	 */
	Thread *myself = Thread::myself();
	if (!myself)
	{
		error(__func__, ": main thread? base ", myself);
		throw -1;
	}

	/*
	 * technically we have 4 stacks:
	 * current stack (rsp) having Thread* _thread inside, prefix c
	 * primary OS stack _thread->_stack - whith the same OS thread info, prefix p
	 * new stack - Thread * _nthread inside to be set to _thread with data, prefix n
	 * new primary os stack _nthread->_stack - not touched
	 */
	addr_t const cstack_top = (addr_t)myself->stack_top();
	addr_t const cstack_base = (addr_t)myself->stack_base();

	/*
	 * some position inside stack after setcontext()
	 */
	addr_t nfrom_stack = (addr_t)getstackaddress(uc);

	/* if our stack to be set is outside current stack
	 * than this could be not the same OS thread (it can have plurality of stacks)
	 */
	if (nfrom_stack < cstack_base || nfrom_stack >= cstack_top)
	{
		/* 
		 * we outside current stack - probalby we will switch to another OS thread
		 * lets check this by understanding Thread* address for new stack and
		 * compare it with myself - by running myself inside function with new stack
		 */
		Thread *new_myself = nullptr;
		getThreadRef(uc, &new_myself);

		// log(__func__, ": cur thread* ", Hex(addr_t(myself)), " stack ", Hex_range(cstack_base, cstack_top - cstack_base),
		// 	" =>nsp ", Hex(nfrom_stack));
		if (new_myself && new_myself != myself)
		{
			/*
			 * need copy stack data because we will switch to another os thread
			 *
			 * !!! FIXME potentially dangerous because any Genode objects/caps
			 * created/accessed from original thread could be not accessible 
			 * (or accessible with different permissions);
			 * need to wrap their callers and take some measures, on case-by-case
			 * eg by redirecting operations with them to original OS thread
			 * 
			 * TBD: need to have a registry and process such objects in the OS thread context
			 */
			// log(__func__, ": old invalid thread* ", Hex(addr_t(new_myself)));

			/*
			 * copy size will be from original beginning of stack structure stored
			 * in the _stack field in the Thread class till the end of stack allocation
			 * stack_base .. stack_top .. _stack .. stack_base+area_size
			 * we need sizeof(Stack) only, while it contain per-OS native 
			 * thread/utcb structures, as well as C++ vtab
			 * fields to copy:
			 * _stack[]				no need to copy
			 * _libc_tls_pointer	no need to copy should be already the same
			 * _name				no need to copy
			 * _thread	need to copy as related to os thread
			 * _base	should not be overrided because it is from current stack
			 * _ds_cap	should not be overrided because it is from current stack
			 * _native_thread	need to copy as related to os thread
			 * _utcb			need to copy as related to os thread
			 * so, technically we have to copy everything after 
			 */
			size_t cend_stack = ((cstack_base) & ~(stack_virtual_size() - 1)) + stack_virtual_size();
			addr_t const cstack_abase = cstack_base & ~(stack_virtual_size() - 1);
			addr_t const nstack_abase = nfrom_stack & ~(stack_virtual_size() - 1);

			/*
			 * address inside primary thread stack
			 * cant use it directly - could be differ from current
			 * even for the same Thread*
			 * use it only to calculate native_off (same for all stacks)
			 */
			addr_t mnative = (addr_t)&myself->native_thread();
			size_t native_off = mnative - (mnative & ~(stack_virtual_size() - 1));

			/* adresses of native_thread field inside stack */
			addr_t cnative = cstack_abase + native_off;
			addr_t new_native = nstack_abase + native_off;

			/* use info about Stack field order from stack.h for Thread& field offset */
			addr_t thr_off = sizeof(Ram_dataspace_capability) + sizeof(addr_t) + sizeof(void *);

			/* copy Thread& field from current stack to new stack */
			*((void **)(new_native - thr_off)) = *((void **)(cnative - thr_off));

			/* copy pthread* filed from current stack to new stack */
			Thread::Stack_info info = Thread::mystack();
			*((void **)(nstack_abase + info.libc_tls_pointer_offset))
				= *((void **)(cstack_abase + info.libc_tls_pointer_offset));

			/* copy rest without last page (could be stack guard if enough space) */
			size_t size = cend_stack - size_t(cnative);
			/* FIXME: linux version does not have guard page in the end while others has */
			size = size > 0x1000 ? size - 0x1000 : size;
			Genode::memcpy((void *)new_native, (void *)cnative, size);
		}
	}

	return setcontext(uc);
}
