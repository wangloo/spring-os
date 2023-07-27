#include <uspace/proc.h>
#include <uspace/uvspace.h>
#include <task.h>
#include <kmem.h>
#include <procinfo.h>
#include <errno.h>
#include <string.h>
#include <sched.h>


int process_task_create_hook(void *item, void *context)
{
	struct process *proc = (struct process *)context;
	struct task *task = (struct task *)item;

	if (!(task->flags & TASK_FLAGS_KERNEL)) {
		task->vs = &proc->vspace;
		task->pid = proc->pid;
		task->state = TASK_STATE_WAIT_EVENT;
	}

	task_stat_init(task);

	return 0;
}

struct process *create_process(int pid, task_func_t func,
		void *usp, int prio, int aff, unsigned long opt)
{
	struct process *proc = NULL;
	struct task *task;
	int ret;

	proc = kalloc(sizeof(struct process));
	if (!proc)
		return NULL;
	
	/*
	 * if the process is not root service, then its right
	 * will be given by root service, when create the process.
	 */
	proc->pid = pid;
	INIT_LIST_HEAD(&proc->task_list);
	// spin_lock_init(&proc->lock); // FIXME

	ret = user_vspace_init(proc);
	if (ret)
		goto vspace_init_fail;
	
	/*
	 * create a root task for this process
	 */
	task = create_task(NULL, func, TASK_STACK_SIZE, usp, prio, aff,
			opt | TASK_FLAGS_NO_AUTO_START | TASK_FLAGS_ROOT, proc);
	if (!task)
		goto task_create_fail;
	
	proc->root_task = task;
	list_add_tail(&task->proc_list, &proc->task_list);
	proc->task_cnt++;

    // TODO
	ret = init_proc_handles(proc);
	if (ret)
		goto handle_init_fail;

	return proc;

handle_init_fail:
    assert(0); // TODO
	//do_release_task(task);
task_create_fail:
	user_vspace_deinit(proc);
vspace_init_fail:
	kfree(proc);

	return NULL;
}


struct process *create_root_process(task_func_t func, void *usp,
		int prio, int aff, unsigned long opt)
{
	struct process *proc;
	struct task_stat *ts;

	proc = create_process(1, func, usp, prio, aff, opt);
	if (!proc)
		return NULL;
	
	ts = get_task_stat(proc->root_task->tid);
	strcpy(ts->cmd, "roots.elf");
	// iqueue_init(&proc->iqueue, 0, &proc->kobj); // FIXME
	// kobject_init(&proc->kobj, KOBJ_TYPE_PROCESS,
	// 		PROC_RIGHT_MASK, (unsigned long)proc);
	proc->flags |= PROC_FLAGS_ROOT | PROC_FLAGS_VMCTL | PROC_FLAGS_HWCTL;
	// proc->kobj.ops = &proc_kobj_ops; // FIXME

	return proc;
}

int wake_up_process(struct process *proc)
{
	int ret = 0;
	struct task *task;

	if (!proc)
		return -EINVAL;
	list_for_each_entry(task, &proc->task_list, proc_list)
		ret +=  __wake_up(task, TASK_STATE_PEND_OK, 0);

	return ret;
}