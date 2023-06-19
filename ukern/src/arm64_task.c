#include <string.h>
#include <asm/arm64_common.h>
#include <task.h>

// 内核栈的顶存储硬件上下文
#define stack_to_gp_regs(base) \
	(gp_regs *)((base) - sizeof(gp_regs))

void arch_init_task(void *task, void *entry, void *user_sp, void *arg)
{
  extern void aarch64_task_exit(void);
  struct task *tsk = (struct task *)task;
  gp_regs *regs = stack_to_gp_regs(tsk->stack_top);

  memset(regs, 0, sizeof(gp_regs));
  tsk->stack_base = (void *)regs;

  regs->sp_el0 = (uint64_t)user_sp;
  regs->elr = (uint64_t)entry;
  regs->x18 = (uint64_t)task;

  if (tsk->flags & TASK_FLAGS_KERNEL) {
    /*
     * if the task is not a deadloop the task will exist
     * by itself like below
     * int main(int argc, char **argv)
     * {
     *	do_some_thing();
     *	return 0;
     * }
     * then the lr register should store a function to
     * handle the task's exist
     *
     * kernel task will not use fpsimd now, so kernel task
     * do not need to save/restore it
     */
    regs->lr = (uint64_t)aarch64_task_exit;
    regs->spsr = AARCH64_SPSR_EL1h;
  } else {
    regs->spsr = AARCH64_SPSR_EL0t;
    tsk->cpu_context.tpidr_el0 = 0;
    tsk->cpu_context.tpidrro_el0 = (uint64_t)tsk->pid << 32 | (tsk->tid);
    tsk->cpu_context.ttbr_el0 = task_ttbr_value(task);
  }
}