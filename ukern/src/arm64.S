#include <asm/asm_macro.S.h>

  .global aarch64_task_exit

func aarch64_task_exit
	mov	x0, x0
	bl	task_exit
1:
	b	1b
endfunc aarch64_task_exit
