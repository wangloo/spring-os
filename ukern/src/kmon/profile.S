#include <asm/asm_macro.S.h>

.macro cyg_profile_enter
	stp	x29, x30, [sp, #-16]!
	mov	x29, sp
.endm

.macro cyg_profile_exit
	ldp	x29, x30, [sp], #16
	ret
.endm

  .global __cyg_profile_func_enter
func __cyg_profile_func_enter
  cyg_profile_enter
  
	// mcount_get_pc	x0		//       function's pc
	// mcount_get_lr	x1		//       function's lr (= parent's pc)
  ldr x2, [x29]       //       function's fp
  bl ftrace_start

  cyg_profile_exit
endfunc __cyg_profile_func_enter

  .global __cyg_profile_func_exit
func __cyg_profile_func_exit
  cyg_profile_enter
  
  ldr x2, [x29]       //       function's fp
  bl ftrace_end
  cyg_profile_exit
endfunc __cyg_profile_func_exit