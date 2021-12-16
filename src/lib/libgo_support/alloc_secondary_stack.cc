/*
 * \brief  setcontext/getcontext/makecontext/swapcontext support library
 * \author Alexander Tormasov
 * \date   2021-03-12
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
