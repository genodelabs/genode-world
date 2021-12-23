/*-
 * Partially derived from
 *
 * $FreeBSD: releng/12.0/sys/amd64/include/asm.h 326123 2017-11-23 11:40:16Z kib $
 */

#ifndef _MACROS_H_
#define	_MACROS_H_

# define C_SYMBOL_NAME(name) name
#ifndef C_LABEL

/* Define a macro we can use to construct the asm name for a C symbol.  */
# define C_LABEL(name)  name##:

#endif

# define CALL_MCOUNT            /* Do nothing.  */

/* generated offsets from mcontext main.cc */
#define oX0       (2*8)
// FXIME: use ELR filed for stored PC ???
#define oPC       (oX0 + 32 * 8)
#define oSP       (oX0 + 31 * 8)
#define oPSTATE   (oX0 + 33 * 8)

#define oX21      (oX0 + 21 * 8)
#define oFP       (oX0 + 29 * 8)
#define oLR       (oX0 + 30 * 8)

#define AARCH64_FP
// FIXME - no magic/header/etc, just point to mc_gpregs.fp_q[0]
// start of mc_fpreg structure from start of ucontext
#define oEXTENSION (oX0 + 34 * 8)
// start of vreg[0] in mc_fpreg structure
#define oV0        (0)
#define oFPSR      (oV0 + 32 * 16)
#define oFPCR      (oFPSR + 4)

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

/* aarch64 specifics */
#define SP_ALIGN_SIZE 15

#define SP_ALIGN_MASK ~15

/* Size of an X regiser in bytes. */
#define SZREG 8

/* Size of a V register in bytes. */
#define SZVREG 16

/* Number of integer parameter passing registers. */
#define NUMXREGARGS 8

/* Number of FP parameter passing registers. */
#define NUMDREGARGS 8

/* Size of named integer argument in bytes when passed on the
   stack.  */
#define SIZEOF_NAMED_INT 4

/* Size of an anonymous integer argument in bytes when passed on the
   stack.  */
#define SIZEOF_ANONYMOUS_INT 8

/* from aarch64/sysdep.h */
#ifdef __LP64__
# define AARCH64_R(NAME)        R_AARCH64_ ## NAME
# define PTR_REG(n)             x##n
# define PTR_LOG_SIZE           3
# define PTR_ARG(n)
# define SIZE_ARG(n)
#else
# define AARCH64_R(NAME)        R_AARCH64_P32_ ## NAME
# define PTR_REG(n)             w##n
# define PTR_LOG_SIZE           2
# define PTR_ARG(n)             mov     w##n, w##n
# define SIZE_ARG(n)            mov     w##n, w##n
#endif

#define PTR_SIZE        (1<<PTR_LOG_SIZE)

/* Syntactic details of assembler.  */

#define ASM_SIZE_DIRECTIVE(name) .size name,.-name

# ifndef JUMPTARGET
#  define JUMPTARGET(sym)       sym
# endif
#  define HIDDEN_JUMPTARGET(name) JUMPTARGET(name)

/* Mark the end of function named SYM.  This is used on some platforms
   to generate correct debugging information.  */
# ifndef END
#  define END(name)   \
   cfi_endproc;      \
   ASM_SIZE_DIRECTIVE(name)
#endif

/* Branch Target Identitication support.  */
#define BTI_C           hint    34
#define BTI_J           hint    36

/* Return address signing support (pac-ret).  */
#define PACIASP         hint    25
#define AUTIASP         hint    29

/* GNU_PROPERTY_AARCH64_* macros from elf.h for use in asm code.  */
#define FEATURE_1_AND 0xc0000000
#define FEATURE_1_BTI 1
#define FEATURE_1_PAC 2

/* Add a NT_GNU_PROPERTY_TYPE_0 note.  */
#define GNU_PROPERTY(type, value)       \
  .section .note.gnu.property, "a";     \
  .p2align 3;                           \
  .word 4;                              \
  .word 16;                             \
  .word 5;                              \
  .asciz "GNU";                         \
  .word type;                           \
  .word 4;                              \
  .word value;                          \
  .word 0;                              \
  .text

/* Add GNU property note with the supported features to all asm code
   where sysdep.h is included.  */
#if HAVE_AARCH64_BTI && HAVE_AARCH64_PAC_RET
GNU_PROPERTY (FEATURE_1_AND, FEATURE_1_BTI|FEATURE_1_PAC)
#elif HAVE_AARCH64_BTI
GNU_PROPERTY (FEATURE_1_AND, FEATURE_1_BTI)
#endif

/* Define an entry point visible from C.  */
#define ENTRY(name)                                             \
  .globl C_SYMBOL_NAME(name);                                   \
  .type C_SYMBOL_NAME(name),%function;                          \
  .p2align 6;                                                   \
  C_LABEL(name)                                                 \
  cfi_startproc;                                                \
  BTI_C;                                                        \
  CALL_MCOUNT


#endif /* _MACROS_H_ */
