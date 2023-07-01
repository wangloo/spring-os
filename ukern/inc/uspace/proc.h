#pragma once

#include <vspace.h>
#include <current.h>
#include <list.h>

#define PROC_FLAGS_VMCTL	(1 << 0)
#define PROC_FLAGS_HWCTL	(1 << 1)
#define PROC_FLAGS_ROOT		(1 << 31)
#define PROC_FLAGS_MASK		(PROC_FLAGS_VMCTL | PROC_FLAGS_HWCTL)

struct process {
	int pid;
	int flags;
	int task_cnt;
	int stopped;

	struct vspace vspace;

	/*
	 * handle_desc_table will store all the kobjects created
	 * and kobjects connected by this process. and
	 *
	 * when close or open kobject, it will only clear or
	 * set the right for related kobject in kobj_table.
	 */
	// struct handle_desc *handle_desc_table;
	struct task *root_task;
	struct list_head task_list;
	//spinlock_t lock;

	// struct kobject kobj;
	// struct iqueue iqueue;
};

#define current_proc()		((struct process *)(current()->vs->pdata))
#define task_to_proc(task)	((struct process *)((task)->vs->pdata))


int process_task_create_hook(void *item, void *context);
int wake_up_process(struct process *proc);