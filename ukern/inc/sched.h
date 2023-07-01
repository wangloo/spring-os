#pragma once
#include <task.h>

static inline void cpu_relax(void)
{
	asm volatile("yield" ::: "memory");
}

void sched(void);
void cond_resched(void);
int sched_init(void);
void sched_local_init(void);
int select_task_run_cpu(void);

void sched_update_sched_timer(void);

int __wake_up(struct task *task, long pend_state, unsigned long data);