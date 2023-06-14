#pragma once

#include <vspace.h>
#include <current.h>

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
	// struct list_head task_list;
	//spinlock_t lock;

	// struct kobject kobj;
	// struct iqueue iqueue;
};