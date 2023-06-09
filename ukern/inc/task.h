#pragma once
#include <task_def.h>

static struct task_info *get_task_info(struct task *task)
{
  return &task->ti;
}
static int inline task_is_idle(struct task *task)
{
  return (task->flags & TASK_FLAGS_IDLE);
}

static inline int get_task_tid(struct task *task)
{
  return task->tid;
}

static inline uint8_t get_task_prio(struct task *task)
{
  return task->prio;
}

static inline int task_is_suspend(struct task *task)
{
  return !!(task->state & TASK_STATE_WAIT_EVENT);
}

static inline int task_is_running(struct task *task)
{
  return (task->state == TASK_STATE_RUNNING);
}

struct task *task_idle_pcpu(int cpuid);
void task_suspend(void);
void task_die(void);
int create_idle_task(void);