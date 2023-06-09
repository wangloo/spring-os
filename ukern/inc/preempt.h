#pragma once
#include <task_info.h>

extern void cond_resched(void);

static inline int preempt_allowed(void)
{
  return !current_task_info()->preempt_count;
}

static inline void preempt_enable(void)
{
  current_task_info()->preempt_count--;
  wmb();
  cond_resched();
}

static void inline preempt_disable(void)
{
  current_task_info()->preempt_count++;
  wmb();
}
