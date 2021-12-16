#include "macros.h"

/*  int __setcontext (const ucontext_t *ucp)

  Restores the machine context in UCP and thereby resumes execution
  in that context.

  This implementation is intended to be used for *synchronous* context
  switches only.  Therefore, it does not have to restore anything
  other than the PRESERVED state.  */
ENTRY(setcontext)
	cfi_startproc
	/* Save argument since syscall will destroy it.  */
	pushq	%rdi
	cfi_adjust_cfa_offset(8)

	/* Pop the pointer into RDX. The choice is arbitrary, but
	   leaving RDI and RSI available for use later can avoid
	   shuffling values.  */
	popq	%rdx
	cfi_adjust_cfa_offset(-8)

	/* Restore the floating-point context.  Not the registers, only the
	   rest.  */
	leaq	OF_mc_fpstate(%rdx), %rcx
	fldenv	(%rcx)
	ldmxcsr OF_oMXCSR(%rdx)


	/* Load the new stack pointer, the preserved registers and
	   registers used for passing args.  */
	cfi_def_cfa(%rdx, 0)
	cfi_offset(%rbx,OF_mc_rbx)
	cfi_offset(%rbp,OF_mc_rbp)
	cfi_offset(%r12,OF_mc_r12)
	cfi_offset(%r13,OF_mc_r13)
	cfi_offset(%r14,OF_mc_r14)
	cfi_offset(%r15,OF_mc_r15)
	cfi_offset(%rsp,OF_mc_rsp)
	cfi_offset(%rip,OF_mc_rip)

	movq	OF_mc_rsp(%rdx), %rsp
	movq	OF_mc_rbx(%rdx), %rbx
	movq	OF_mc_rbp(%rdx), %rbp
	movq	OF_mc_r12(%rdx), %r12
	movq	OF_mc_r13(%rdx), %r13
	movq	OF_mc_r14(%rdx), %r14
	movq	OF_mc_r15(%rdx), %r15

	/* The following ret should return to the address set with
	getcontext.  Therefore push the address on the stack.  */
	movq	OF_mc_rip(%rdx), %rcx
	pushq	%rcx

	movq	OF_mc_rsi(%rdx), %rsi
	movq	OF_mc_rdi(%rdx), %rdi
	movq	OF_mc_rcx(%rdx), %rcx
	movq	OF_mc_r8(%rdx), %r8
	movq	OF_mc_r9(%rdx), %r9

	/* Setup finally %rdx.  */
	movq	OF_mc_rdx(%rdx), %rdx

	/* End FDE here, we fall into another context.  */
	cfi_endproc
	cfi_startproc

	/* Clear rax to indicate success.  */
	xorl	%eax, %eax
	ret
	cfi_endproc
END(setcontext)

.section .note.GNU-stack,"",%progbits
