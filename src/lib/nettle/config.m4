define(`srcdir', ``.'')dnl
define(`SYMBOL_PREFIX', `'`$1')dnl
define(`ELF_STYLE', `yes')dnl
define(`COFF_STYLE', `no')dnl
define(`TYPE_FUNCTION', `%function')dnl
define(`TYPE_PROGBITS', `%progbits')dnl
define(`ALIGN_LOG', `no')dnl
define(`W64_ABI', `no')dnl
define(`RODATA', `.section .rodata')dnl
define(`WORDS_BIGENDIAN', `no')dnl
define(`ASM_X86_ENDBR',`endbr64')dnl
define(`ASM_X86_MARK_CET_ALIGN',`3')dnl
define(`ASM_PPC_WANT_R_REGISTERS',`n/a')dnl
divert(1)

	.pushsection ".note.gnu.property", "a"
	.p2align ASM_X86_MARK_CET_ALIGN
	.long 1f - 0f
	.long 4f - 1f
	.long 5
0:
	.asciz "GNU"
1:
	.p2align ASM_X86_MARK_CET_ALIGN
	.long 0xc0000002
	.long 3f - 2f
2:
	.long 3
3:
	.p2align ASM_X86_MARK_CET_ALIGN
4:
	.popsection
.section .note.GNU-stack,"",TYPE_PROGBITS
divert
