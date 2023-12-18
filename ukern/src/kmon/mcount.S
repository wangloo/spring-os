#include <asm/asm_macro.S.h>



	.macro mcount_adjust_addr rd, rn
	sub	\rd, \rn, #8
	.endm

.macro mcount_enter
	stp	x29, x30, [sp, #-16]!
	mov	x29, sp
.endm

.macro mcount_exit
	ldp	x29, x30, [sp], #16
	ret
.endm

// Function's lr
.macro mcount_get_lr reg
	ldr	\reg, [x29]
	ldr	\reg, [\reg, #8]
	mcount_adjust_addr	\reg, \reg
.endm

// Function's pc
.macro mcount_get_pc reg
	ldr	\reg, [x29, #8]
	mcount_adjust_addr	\reg, \reg
.endm


/*
 * void _mcount(unsigned long return_address)
 * @return_address: return address to instrumented function
 *
 * This function makes calls, if enabled, to:
 *     - tracer function to probe instrumented function's entry,
 *     - ftrace_graph_caller to set up an exit hook
 */
  .global _mcount
func _mcount
  mcount_enter
  
	mcount_get_pc	x0		//       function's pc
	mcount_get_lr	x1		//       function's lr (= parent's pc)
  bl functrace

  mcount_exit
endfunc _mcount