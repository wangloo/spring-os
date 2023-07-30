#include <asm/arm64_common.h>
#include <arm64_sysreg.h>
#include <task.h>
#include <kernel.h>

// 内核栈top存储硬件上下文
#define stack_to_gp_regs(base) \
	(gp_regs *)((base) - sizeof(gp_regs))

void arch_init_task(struct task *task, void *entry, 
                      void *user_sp, void *arg)
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
  } 
  else {
    
    regs->spsr = AARCH64_SPSR_EL0t;
    tsk->cpu_context.tpidr_el0 = 0;
    tsk->cpu_context.tpidrro_el0 = (uint64_t)tsk->pid << 32 | (tsk->tid);
    tsk->cpu_context.ttbr_el0 = (uint64_t)task_ttbr_value(tsk);
        
    printf("task: %s's stack_base: 0x%lx\n", tsk->name, tsk->stack_base);
    printf("addr of regs->spsr: 0x%lx\n", regs->spsr);

  }
  
}

void arch_task_sched_in(struct task *task)
{
	struct cpu_context *c = &task->cpu_context;
  
	if (task->flags & TASK_FLAGS_KERNEL)
    return;

	write_sysreg(c->tpidr_el0, TPIDR_EL0);
	write_sysreg(c->tpidrro_el0, TPIDRRO_EL0);

	write_sysreg(c->ttbr_el0, TTBR0_EL1);
}

void arch_set_task_user_stack(struct task *task, unsigned long stack)
{
  gp_regs *regs = stack_to_gp_regs(task->stack_top);
  regs->sp_el0 = stack;
}

void arch_set_task_entry_point(struct task *task, long entry)
{
  gp_regs *regs = stack_to_gp_regs(task->stack_top);
  regs->elr = entry;
  printf("$$ addr of regs->elf: 0x%lx, val: 0x%lx\n", &regs->elr, regs->elr);
}