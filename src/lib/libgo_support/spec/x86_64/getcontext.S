/*
   Copyright (C) 2002-2021 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>, 2002.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include "macros.h"

ENTRY(getcontext)
	/* Save the preserved registers, the registers used for passing
	   args, and the return address.  */
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
	fldenv	(%rcx)
	stmxcsr OF_oMXCSR(%rdi)
	/* All done, return 0 for success.  */
	xorl	%eax, %eax
	ret
END(getcontext)

.section .note.GNU-stack,"",%progbits
