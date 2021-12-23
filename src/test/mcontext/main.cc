/*
 * \brief  setcontext/getcontext/makecontext/swapcontext simple test
 * \author Alexander Tormasov
 * \date   2021-03-12
 */


#include <alloc_secondary_stack.h>


/* libC includes */
extern "C" {
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/limits.h>
#include <sys/random.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <time.h>
#include <time.h>
#include <unistd.h>
#include <ucontext.h>
}
#if 0
#define AARCH64
#include <stddef.h>
ucontext_t a;
#define Pof(name) printf("#define OF_" #name " %d\t/* %d + %d */\n",                     \
						offsetof(ucontext_t, uc_mcontext) + offsetof(mcontext_t, name), \
						offsetof(ucontext_t, uc_mcontext),                              \
						offsetof(mcontext_t, name))
#define Po(name) printf("#define OF_" #name "\t%d\n",                     \
						offsetof(ucontext_t, uc_mcontext) + offsetof(mcontext_t, name))

#define Pom(name,m,pname) printf("#define OF_" #pname "\t%d\n", \
						m+offsetof(ucontext_t, uc_mcontext) + offsetof(mcontext_t, name))

int main(int argc, char **argv)
{
	printf("--- mcontext offset ---\n");
#ifndef AARCH64
	Po(mc_rbx);
	Po(mc_rbp);
	Po(mc_r8);
	Po(mc_r9);
	Po(mc_r10);
	Po(mc_r11);
	Po(mc_r12);
	Po(mc_r13);
	Po(mc_r14);
	Po(mc_r15);
	Po(mc_rdi);
	Po(mc_rsi);
	Po(mc_rdx);
	Po(mc_rcx);
	Po(mc_rip);
	Po(mc_rsp);

	Po(mc_fpstate);
	// taken from linux as oMXCSR == 336 and base oFPREGSMEM == 312
	Pom(mc_fpstate, 24, oMXCSR);
#else
// aarch64
#define Pa(n1, n2, name) printf("#define " #name "\t%d\n", __builtin_offsetof(n1,n2))

#define ucontext(member) offsetof(ucontext_t, member)
#define stack(member) ucontext(uc_stack.member)
#define mcontext(member) ucontext(uc_mcontext.mc_gpregs.member)

#define oX0 mcontext(gp_x)
	printf("%d\n", oX0); // oX0

#define oPC mcontext(gp_lr)
	printf("%d\n", oPC); // oPC

#define oSP mcontext(gp_sp)
	printf("%d\n", oSP); // oSP

#define oPSTATE mcontext(gp_spsr)
	printf("%d\n", oPSTATE); // oPSTATE

#define oEXTENSION ucontext(uc_mcontext.mc_fpregs.fp_q)
	printf("%d\n", oEXTENSION); // oEXTENSION emulation, or !!! need to add 1 page to ucontext!

#define fpsimd_context(member) ucontext(uc_mcontext.mc_fpregs.member)

#define oFPSR fpsimd_context(fp_sr)
	printf("%d\n", oFPSR);		// oFPSR

#define oFPCR fpsimd_context(fp_cr)
	printf("%d\n", oFPCR);		// oFPCR

#endif

	return 0;
}
#else
#define MEM 64000
ucontext_t T1, T2, Main, a;

void fn1()
{
	printf("this is from 1\n");
	setcontext(&Main);
}

void fn2()
{
	printf("this is from 2\n");
	setcontext(&a);
	printf("finished 1\n");
}

void start()
{
	getcontext(&a);
	a.uc_link = 0;
	a.uc_stack.ss_sp = alloc_secondary_stack("test",MEM);
	a.uc_stack.ss_size = MEM;
	a.uc_stack.ss_flags = 0;
	makecontext(&a, &fn1, 0);
}

int main(int argc, char **argv)
{
	printf("--- mcontext test ---\n");
	printf("start... ucontext_t size: %ld\n", sizeof(a.uc_mcontext));
	a.uc_mcontext.mc_len =
		T1.uc_mcontext.mc_len =
			T2.uc_mcontext.mc_len =
				Main.uc_mcontext.mc_len = sizeof(a.uc_mcontext);
	start();
	printf("context prepared for func: %p\n",
		//    a.uc_mcontext.mc_gpregs.gp_lr); // aarch64
										       a.uc_mcontext.mc_rip);
	getcontext(&Main);
	getcontext(&T1);
	T1.uc_link = 0;
	T1.uc_stack.ss_sp = alloc_secondary_stack("test",MEM);
	T1.uc_stack.ss_size = MEM;
	T1.uc_stack.ss_flags = 0;
	makecontext(&T1, &fn1, 0);
	printf("swapcontext stacks: %p to %p\n", Main.uc_stack.ss_sp, T1.uc_stack.ss_sp);
	swapcontext(&Main, &T1);
	printf("swapcontext done\n");
	getcontext(&T2);
	T2.uc_link = 0;
	T2.uc_stack.ss_sp = alloc_secondary_stack("test",MEM);
	T2.uc_stack.ss_size = MEM;
	T2.uc_stack.ss_flags = 0;
	makecontext(&T2, &fn2, 0);
	swapcontext(&Main, &T2);
	printf("completed\n");
	exit(0);
}
#endif
