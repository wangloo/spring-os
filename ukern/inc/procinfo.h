#pragma once

#include <uapi/procinfo.h>

struct task;

void task_stat_init(struct task *task);
void task_stat_release(int tid);
void task_stat_update(struct task *task);
struct task_stat *get_task_stat(int tid);
int procinfo_init(void);