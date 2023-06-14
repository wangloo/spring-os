#pragma once

static inline void *arch_current_task(void)
{
  void *tsk;
  __asm__ volatile("mov %0, x18" : "=r"(tsk));
  return tsk;
}

static inline void *arch_current_task_info(void)
{
  void *tsk_info;
  __asm__ volatile("mov %0, x18" : "=r"(tsk_info));
  return tsk_info;
}

static inline void arch_set_current_task(void *task)
{
  __asm__ volatile("mov x18, %0" : : "r"(task));
}

// TODO: 或许不应该放在这?
static inline void arch_sys_sched(void)
{
  __asm__ volatile("svc #0");
}