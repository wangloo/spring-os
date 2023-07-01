#pragma once
#include <bitmap.h>
#include <task_def.h>

extern bitmap_t tid_map[bits_to_long(OS_NR_TASKS)];

void release_tid(int tid);
int alloc_tid(void);
