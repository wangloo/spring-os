#ifndef _SPRINGOS_ASM_MACRO_H_
#define _SPRINGOS_ASM_MACRO_H_

.macro func _name, align=2
.cfi_sections .debug_frame
.section __asm_code, "ax"
.type \_name, %function
.func \_name
.cfi_startproc
.align \align
\_name:
.endm

.macro endfunc _name
.endfunc
.cfi_endproc
.size \_name, . - \_name
.endm


.macro kfunc _name, align=2
.cfi_sections .debug_frame
.section .text, "ax"
.type \_name, %function
.func \_name
.cfi_startproc
.align \align
\_name:
.endm

.macro endkfunc _name
.endfunc
.cfi_endproc
.size \_name, . - \_name
.endm
#endif
