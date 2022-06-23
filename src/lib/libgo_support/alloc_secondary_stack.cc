/*
 * \brief  setcontext/getcontext/makecontext/swapcontext support library
 * \author Alexander Tormasov
 * \date   2021-03-12
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

#include "alloc_secondary_stack.h"

/* need some space for Stack* structure as a part of stack */
enum { ADDON_SIZE = 128 };

extern "C"
void *alloc_secondary_stack(char const *name, unsigned long stack_size)
{
	Genode::Thread *myself = Genode::Thread::myself();
	if (!myself)
		return nullptr;

	void *ret = myself->alloc_secondary_stack(name, stack_size + ADDON_SIZE);
	if (!ret)
		return nullptr;

	char *c = reinterpret_cast<char *>(ret);
	/* stack top is cleared by ABI-specific init_stack() */
	Genode::memset(c - stack_size, 0, stack_size);

	return (void *)(c - stack_size);
}

extern "C" 
void free_secondary_stack(void *stack)
{
	Genode::Thread *myself = Genode::Thread::myself();
	if (!myself)
		return;
	myself->free_secondary_stack(stack);
}
