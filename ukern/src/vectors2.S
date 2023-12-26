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


// Set sp to interrupt stack top(low)
.macro SWITCHTO_INTR_STACK, tmp0
	mrs	\tmp0, TPIDR_EL1
	ldr	\tmp0, [\tmp0, #PCPU_STACK_OFFSET]
	mov	sp, \tmp0
.endm

// Set sp to process stack base(high)
.macro SWITCHTO_PROC_STACK, tmp0
	ldr	\tmp0, [x18, #PROC_STACK_OFFSET]
	add	sp, \tmp0, 0x2000
.endm



func __sync_from_current_el
  SAVE_ECTX
  mov x0, sp   // function paramter
  bl sync_from_current_el
  
  LOAD_ECTX
  eret
endfunc __sync_from_current_el



func __sync_from_lower_el
  // Change: Save context to whatever kernel stack
  // And copy them to current proc's context later

  // SWITCHTO_PROC_STACK x19
  SAVE_ECTX
  
  ldr x0, [x18, #PROC_STACK_OFFSET]
  add x0, x0, 0x2000
  sub x0, x0, #ECTX_SIZE
  mov x1, sp
  mov x2, #ECTX_SIZE
  bl memcpy
  
  SWITCHTO_INTR_STACK x1
  add sp, sp, #0x2000
  bl sync_from_lower_el

  SWITCHTO_PROC_STACK x1
  sub sp, sp, #ECTX_SIZE
  LOAD_ECTX
  eret
endfunc __sync_from_lower_el




// IRQ is DISABLE in kernel, so this handler
// only trigger for starting scheduler.
// Will not back, context doesn't matter 
func __irq_from_current_el
  SAVE_ECTX

  // Process switch could happen before exception return
  // So must update the current process's kernel stack pointer 
  // make sure context can be restored when this process is back
  // mov     x0, sp
  // str     x0, [x18, #PROC_STACK_OFFSET]	

  // Switch to private interrupt stack for each CPU
  // We can call C function after here
  // SWITCHTO_INTR_STACK x0

  mov x0, sp   // function paramter
  bl irq_from_current_el

  // Before return from interrupt, always check 
  // whether need to resched. x18 will the next process
	// bl sched

  // ldr x1, [x18, PROC_PAGETABLE_OFFSET]
  // msr ttbr0_el1, x1

  // Switch to new process's kernel stack
  // Not load context before IRQ, but new process's 
  // user context
  // SWITCHTO_PROC_STACK x0
  // sub sp, sp, ECTX_SIZE
	LOAD_ECTX
  // sub sp, sp, ECTX_SIZE
	eret
endfunc __irq_from_current_el



func __irq_from_lower_el
  SAVE_ECTX

  bl irq_from_lower_el

  LOAD_ECTX
  eret
endfunc __irq_from_lower_el