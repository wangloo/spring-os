#pragma once
#include <compiler.h>
#include <ctx.h>


#define TASK_STATE_RUNNING    0x00
#define TASK_STATE_READY      0x01
#define TASK_STATE_WAIT_EVENT 0x02
#define TASK_STATE_WAKING     0x04
#define TASK_STATE_SUSPEND    0x08
#define TASK_STATE_STOP       0x10


#define TASK_NAME_SIZE		(32)



struct tcb {
  char name[TASK_NAME_SIZE];

  struct vspace *vs;		// the virtual memory space of this task.

  int prio;

  unsigned long flags;
};
