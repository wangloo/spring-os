#pragma once

#include <arm64_task.h>

static inline struct task *current_task(void)
{
  return (struct task *)arch_current_task();
}

static inline struct task_info *current_task_info(void)
{
  return (struct task_info *)arch_current_task_info();
}

static inline void set_current_task(struct task *task)
{
	arch_set_current_task(task);
}
#define current()           current_task()
#define current_task_info() current_task_info()
#define current_pid()       current()->pid
#define current_tid()       current()->tid
#define current_name()      current()->name
#define current_user_regs() current()->user_regs