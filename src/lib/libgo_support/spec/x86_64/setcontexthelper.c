/*
 * \brief  Implementation of some mach-dependent auxilary functions for innosetcontext for golang runtime 
 * \author Alexander Tormasov
 * \date   2021-12-17
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