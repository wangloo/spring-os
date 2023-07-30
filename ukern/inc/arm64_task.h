/**
 * @file  arm64_task.h
 * @brief 架构相关的task功能实现
 * @level arch-related， 该头文件相对独立
 * @date 2023-07-29
 */
#pragma once

struct task;

static inline void *arch_current_task(void)
{
  void *task;
  __asm__ volatile("mov %0, x18" : "=r"(task));
  return task;
}

static inline void *arch_current_task_info(void)
{
  void *task_info;
  __asm__ volatile("mov %0, x18" : "=r"(task_info));
  return task_info;
}

static inline void arch_set_current_task(struct task *task)
{
  __asm__ volatile("mov x18, %0" : : "r"(task));
}

// TODO: 或许不应该放在这?
static inline void arch_sys_sched(void)
{
  __asm__ volatile("svc #0");
}


void arch_init_task(struct task *task, void *entry, 
                      void *user_sp, void *arg);
void arch_task_sched_in(struct task *task);
void arch_set_task_entry_point(struct task *task, long entry);
void arch_set_task_user_stack(struct task *task, unsigned long stack);