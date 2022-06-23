/*-
 * Partially derived from
 *
 * $FreeBSD: releng/12.0/sys/amd64/include/asm.h 326123 2017-11-23 11:40:16Z kib $
 */

#ifndef _MACROS_H_
#define	_MACROS_H_

#define CNAME(csym)		csym

#define _START_ENTRY	.text; .p2align 4,0x90

#define ENTRY(x)	_START_ENTRY; \
			.globl CNAME(x); .type CNAME(x),@function; CNAME(x):

#define	END(x)		.size x, . - x

/* generated offsets from mcontext main.cc */
#define OF_mc_rbx 80
#define OF_mc_rbp 88
#define OF_mc_r8 56
#define OF_mc_r9 64
#define OF_mc_r10 96
#define OF_mc_r11 104
#define OF_mc_r12 112
#define OF_mc_r13 120
#define OF_mc_r14 128
#define OF_mc_r15 136
#define OF_mc_rdi 24
#define OF_mc_rsi 32
#define OF_mc_rdx 40
#define OF_mc_rcx 48
#define OF_mc_rip 176
#define OF_mc_rsp 200
#define OF_mc_fpstate 240
#define OF_oMXCSR 264

/* Makros to generate eh_frame unwind information.  */
#define cfi_startproc .cfi_startproc
#define cfi_endproc .cfi_endproc
#define cfi_def_cfa(reg, off) .cfi_def_cfa reg, off
#define cfi_def_cfa_register(reg) .cfi_def_cfa_register reg
#define cfi_def_cfa_offset(off) .cfi_def_cfa_offset off
#define cfi_adjust_cfa_offset(off) .cfi_adjust_cfa_offset off
#define cfi_offset(reg, off) .cfi_offset reg, off
#define cfi_rel_offset(reg, off) .cfi_rel_offset reg, off
#define cfi_register(r1, r2) .cfi_register r1, r2
#define cfi_return_column(reg) .cfi_return_column reg
#define cfi_restore(reg) .cfi_restore reg
#define cfi_same_value(reg) .cfi_same_value reg
#define cfi_undefined(reg) .cfi_undefined reg
#define cfi_remember_state .cfi_remember_state
#define cfi_restore_state .cfi_restore_state
#define cfi_window_save .cfi_window_save
#define cfi_personality(enc, exp) .cfi_personality enc, exp
#define cfi_lsda(enc, exp) .cfi_lsda enc, exp

#endif /* _MACROS_H_ */
