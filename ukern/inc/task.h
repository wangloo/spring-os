/**
 * @file task.h
 * @brief 提供向上提供的所有接口
 * @level 外部只需要include这一个头文件即可
 * @date 2023-07-30
 */
#pragma once
#include <task_def.h>
#include <current.h>
#include <task_info.h>
#include <kernel.h>


//=====================================
//getter/setter
//=====================================
static struct task_info *task_info(struct task *task)
{
  return &task->ti;
}

static int inline task_is_idle(struct task *task)
{
  return (task->flags & TASK_FLAGS_IDLE);
}

static inline int task_tid(struct task *task)
{
  return task->tid;
}

static inline uint8_t task_prio(struct task *task)
{
  return task->prio;
}

static inline paddr_t task_ttbr_value(struct task *task)
{
	struct vspace *vs = task->vs;

  assert(!(task->flags & TASK_FLAGS_KERNEL));
  // printf("%p : 0x%lx\n", vs->pgdp, vs->asid); 
  // printf("vs->asid: %d\n", vs->asid);
	return (paddr_t)vtop(vs->pgdp) | ((paddr_t)vs->asid << 48);
}

static inline int task_is_suspend(struct task *task)
{
  return !!(task->state & TASK_STATE_WAIT_EVENT);
}

static inline int task_is_running(struct task *task)
{
  return (task->state == TASK_STATE_RUNNING);
}

static inline int task_is_need_resched(struct task *task)
{
	return (task->ti.flags & TIF_NEED_RESCHED);
}

static inline void task_clear_resched(struct task *task)
{
	task->ti.flags &= ~TIF_NEED_RESCHED;
}

static inline void task_set_resched(struct task *task)
{
	task->ti.flags |= TIF_NEED_RESCHED;
}


struct task *task_idle_pcpu(int cpuid);
void task_suspend(void);
void task_die(void);
int task_ready(struct task *task, int preempt);
int task_wakeup(struct task *task, long pend_state, unsigned long data);
int create_idle_task(void);
struct task *create_kthread(char *name, 
                            task_func_t func, 
                            int prio,
                            int aff, 
                            unsigned long opt, 
                            void *arg);
struct task *create_task(char *name,
                         task_func_t func,
                         size_t stk_size,
                         void *usp,
                         int prio,
                         int aff,
                         unsigned long opt,
                         void *arg);
void do_for_all_task(void (*hdl)(struct task *task));