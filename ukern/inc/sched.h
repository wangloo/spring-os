#pragma once


void sched(void);
void cond_resched(void);
int sched_init(void);
void sched_local_init(void);
int select_task_run_cpu(void);

void sched_update_sched_timer(void);