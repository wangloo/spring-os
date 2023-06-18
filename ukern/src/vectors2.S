#include <asm/asm_macro.S.h>
#include <asm/arm64_default.h>
#include <asm/asm_offset.h>
#include <asm/context.S.h>
#include <config/config.h>
#include <task_info.h>

.global __bad_mode
.global __sync_from_lower_el
.global __sync_from_current_el
.global __irq_from_lower_el
.global __irq_from_current_el


func __bad_mode
    b    .
endfunc __bad_mode


.macro LOAD_PCPU_STACK, tmp0
	mrs	\tmp0, TPIDR_EL1
	ldr	\tmp0, [\tmp0, #PCPU_STACK_OFFSET]
	mov	sp, \tmp0
.endm

func exception_return
	LOAD_PCPU_STACK x1			// load percpu stack, need ensure the irq is off.

	bl	sched_prepare	   // check whether need to resched. x18 will the next task.

	ldr	x1, [x18, #TASK_STACK_OFFSET]	// load the running task's stack
	mov	sp, x1				// change to the new stack address

	ldr	x1, [sp, #GP_REGS_SPSR_OFFSER]			// load spsr
	and	x1, x1, #0x0f
	cmp	x1, #ARM64_SPSR_KERNEL		// whether the task will return to user
	b.eq	__do_exception_return

	mov	x0, sp
	bl	task_return_to_user
	str	xzr, [x18, #TASK_USER_REGS_OFFSET] // clear the user_regs for task

__do_exception_return:
	LOAD_GP_REGS
	eret
endfunc exception_return


func __sync_from_current_el
  SAVE_GP_REGS
  SAVE_OTHER_REGS
  mov x0, sp   // function paramter
  bl sync_from_current_el

  LOAD_GP_REGS
  eret
endfunc __sync_from_current_el




func __sync_from_lower_el
  SAVE_GP_REGS
  mov x0, sp   // function paramter

  LOAD_GP_REGS
  eret
endfunc __sync_from_lower_el




func __irq_from_current_el
  SAVE_GP_REGS

  // Set the irq flags into ti->flags.
  ldr	x1, [x18, #TASK_INFO_FLAGS_OFFSET]
  orr	x1, x1, #__TIF_HARDIRQ_MASK
  str	x1, [x18, #TASK_INFO_FLAGS_OFFSET]
  dsb	sy

  mov x0, sp
  str     x0, [x18, #TASK_STACK_OFFSET]	// store the stack in case this task will scheded out.
  bl irq_from_current_el

  // clear the irq flags into ti->flags.
  ldr	x1, [x18, #TASK_INFO_FLAGS_OFFSET]
  and	x1, x1, #(~__TIF_HARDIRQ_MASK)
  str	x1, [x18, #TASK_INFO_FLAGS_OFFSET]
  dsb	sy
    
  b exception_return
endfunc __irq_from_current_el





func __irq_from_lower_el
  SAVE_GP_REGS
  mov x0, sp

  LOAD_GP_REGS
  eret
endfunc __irq_from_lower_el

