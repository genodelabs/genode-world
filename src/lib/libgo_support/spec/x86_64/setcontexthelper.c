/*
 * \brief  Implementation of some mach-dependent auxilary functions for innosetcontext for golang runtime 
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

#include <sys/ucontext.h>

/*
 * get platform-dependent stack address from registed
 */
long getstackaddress(ucontext_t *uc)
{
	/*
	 * some position inside stack after setcontext()
	 */
	return uc->uc_mcontext.mc_rsp;
}

/*
 * init platform-dependent ucontext_t
 */
void initucontext(ucontext_t *uc)
{
	uc->uc_mcontext.mc_len = sizeof(uc->uc_mcontext);
}

void othercontext(void **pmyself);

/*
 * obtain Thread* address for new stack
 * by running myself inside function with new stack
 */
void getThreadRef(ucontext_t *uc, void *pnew_myself)
{
	ucontext_t T1, Main;
	T1.uc_mcontext.mc_len =
		Main.uc_mcontext.mc_len = sizeof(Main.uc_mcontext);

	/* execute function othercontext() in another stack using local var pointer */
	getcontext(&T1);
	T1.uc_link = &Main;
	T1.uc_stack.ss_sp = (void *)(uc->uc_mcontext.mc_rsp - MINSIGSTKSZ);
	T1.uc_stack.ss_size = MINSIGSTKSZ;
	makecontext(&T1, (void (*)()) & othercontext, 1, pnew_myself);
	swapcontext(&Main, &T1);
}