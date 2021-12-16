#include "macros.h"

/* int __swapcontext (ucontext_t *oucp, const ucontext_t *ucp);

  Saves the machine context in oucp such that when it is activated,
  it appears as if __swapcontextt() returned again, restores the
  machine context in ucp and thereby resumes execution in that
  context.

  This implementation is intended to be used for *synchronous* context
  switches only.  Therefore, it does not have to save anything
  other than the PRESERVED state.  */

ENTRY(swapcontext)
	cfi_startproc
	/* Save the preserved registers, the registers used for passing args,
	   and the return address.  */
	movq	%rbx, OF_mc_rbx(%rdi)
	movq	%rbp, OF_mc_rbp(%rdi)
	movq	%r12, OF_mc_r12(%rdi)
	movq	%r13, OF_mc_r13(%rdi)
	movq	%r14, OF_mc_r14(%rdi)
	movq	%r15, OF_mc_r15(%rdi)

	movq	%rdi, OF_mc_rdi(%rdi)
	movq	%rsi, OF_mc_rsi(%rdi)
	movq	%rdx, OF_mc_rdx(%rdi)
	movq	%rcx, OF_mc_rcx(%rdi)
	movq	%r8, OF_mc_r8(%rdi)
	movq	%r9, OF_mc_r9(%rdi)

	movq	(%rsp), %rcx
	movq	%rcx, OF_mc_rip(%rdi)
	leaq	8(%rsp), %rcx		/* Exclude the return address.  */
	movq	%rcx, OF_mc_rsp(%rdi)

	/* We have separate floating-point register content memory on the
	   stack.  We use the __fpregs_mem block in the context.  Set the
	   links up correctly.  */

	leaq	OF_mc_fpstate(%rdi), %rcx

	/* Save the floating-point environment.  */
	fnstenv	(%rcx)
	stmxcsr OF_oMXCSR(%rdi)


	/* move second parameter address to rdx to keep compliance with original */
	movq	%rsi, %rdx

	/* Restore the floating-point context.  Not the registers, only the
	   rest.  */
	leaq	OF_mc_fpstate(%rdx), %rcx
	fldenv	(%rcx)
	ldmxcsr OF_oMXCSR(%rdx)

	/* Load the new stack pointer and the preserved registers.  */
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

	/* Setup registers used for passing args.  */
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
END(swapcontext)

.section .note.GNU-stack,"",%progbits
