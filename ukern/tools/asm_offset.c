// #include <task_def.h>
// #include <pcpu.h>
#include <ctx_arm64.h>
#include <proc.h>
#include <cpu.h>

#define DEFINE(sym, val) \
	asm volatile("\n->" #sym " %0 " #val : : "i" (val))

int main(void)
{
//   DEFINE(TASK_INFO_SIZE, sizeof(struct task_info));
//   DEFINE(TASK_INFO_FLAGS_OFFSET, offsetof(struct task_info, flags));
//   DEFINE(PCPU_SIZE, sizeof(struct pcpu));
//   DEFINE(PCPU_ID_OFFSET, offsetof(struct pcpu, pcpu_id));
//   DEFINE(PCPU_STACK_OFFSET, offsetof(struct pcpu, stack));
//   DEFINE(TASK_SIZE, sizeof(struct task));
//   DEFINE(TASK_STACK_OFFSET, offsetof(struct task, stack_base));
//   DEFINE(PCPU_CURRENT_TASK, offsetof(struct pcpu, running_task));
//   DEFINE(GP_REGS_SIZE, sizeof(gp_regs));
//   DEFINE(TASK_USER_REGS_OFFSET, offsetof(struct task, user_regs));
// 	DEFINE(GP_REGS_SPSR_OFFSER, offsetof(gp_regs, spsr));
    DEFINE(ECTX_SIZE, sizeof(struct econtext));
    DEFINE(CTX_SIZE, sizeof(struct context));
    DEFINE(GP_REGS_SIZE, sizeof(struct gp_regs));
    DEFINE(PROC_STACK_OFFSET, offsetof(struct proc, stack_base));
    DEFINE(PROC_PAGETABLE_OFFSET, offsetof(struct proc, pagetable));
    DEFINE(PCPU_STACK_OFFSET, offsetof(struct cpu, intr_stack));

  return 0;
}