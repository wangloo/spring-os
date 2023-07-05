#include <asm/asm_macro.S.h>

  .global aarch64_task_exit

kfunc aarch64_task_exit
	mov	x0, x0
	bl	task_exit
1:
	b	1b
endkfunc aarch64_task_exit
