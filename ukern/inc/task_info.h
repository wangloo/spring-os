#pragma once

#include <utils.h>
#define TIF_NEED_RESCHED       0
#define TIF_32BIT              1
#define TIF_DONOT_PREEMPT      2
#define TIF_TICK_EXHAUST       3
#define TIF_IN_USER            4
#define TIF_HARDIRQ_MASK       8
#define TIF_SOFTIRQ_MASK       9
#define TIF_NEED_STOP          10
#define TIF_NEED_FREEZE        11
#define TIF_WAIT_INTERRUPTED   12

#define __TIF_NEED_RESCHED     (UL(1) << TIF_NEED_RESCHED)
#define __TIF_32BIT            (UL(1) << TIF_32BIT)
#define __TIF_DONOT_PREEMPT    (UL(1) << TIF_DONOT_PREEMPT)
#define __TIF_TICK_EXHAUST     (UL(1) << TIF_TICK_EXHAUST)
#define __TIF_IN_USER          (UL(1) << TIF_IN_USER)
#define __TIF_HARDIRQ_MASK     (UL(1) << TIF_HARDIRQ_MASK)
#define __TIF_SOFTIRQ_MASK     (UL(1) << TIF_SOFTIRQ_MASK)
#define __TIF_NEED_STOP        (UL(1) << TIF_NEED_STOP)
#define __TIF_NEED_FREEZE      (UL(1) << TIF_NEED_FREEZE) // only used for VCPU.
#define __TIF_WAIT_INTERRUPTED (UL(1) << TIF_WAIT_INTERRUPTED)

#define __TIF_IN_INTERRUPT     (__TIF_HARDIRQ_MASK | __TIF_SOFTIRQ_MASK)


#ifndef __ASSEMBLY__
#include <current.h>
#include <barrier.h>

#define TASK_INFO_INIT(__ti, task) 		\
	do {					\
		(__ti)->preempt_count = 0; 	\
		(__ti)->flags = 0;		\
	} while (0)

/*
 * this task_info is stored at the top of the task's
 * stack
 */
struct task_info {
  int preempt_count;
  unsigned long flags;
};

static inline void set_need_resched(void)
{
  current_task_info()->flags |= __TIF_NEED_RESCHED;
  wmb();
}

static inline void clear_need_resched(void)
{
  current_task_info()->flags &= ~__TIF_NEED_RESCHED;
  wmb();
}

static inline void clear_do_not_preempt(void)
{
  current_task_info()->flags &= ~__TIF_DONOT_PREEMPT;
  wmb();
}

static inline int need_resched(void)
{
  return !!(current_task_info()->flags & __TIF_NEED_RESCHED);
}

static inline void do_not_preempt(void)
{
  current_task_info()->flags |= __TIF_DONOT_PREEMPT;
  wmb();
}

static inline int in_interrupt(void)
{
  return (current_task_info()->flags & __TIF_IN_INTERRUPT);
}



#endif