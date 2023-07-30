/**
 * @file current.h
 * @brief 
 * @level 仅task子系统内部可见，外部include task.h即可访问
 * @date 2023-07-30
 */
#pragma once

#include <arm64_task.h>

struct task_info;

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

// 应该在task.c或task.h中实现
// #define current_pid()       current()->pid
// #define current_tid()       current()->tid
// #define current_name()      current()->name
// #define current_user_regs() current()->user_regs
