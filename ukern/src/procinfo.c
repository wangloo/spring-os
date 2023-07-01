#include <uspace/proc.h>
#include <procinfo.h>
#include <task.h>
#include <page.h>
#include <assert.h>
#include <string.h>

// struct kobject *task_stat_pma;
static struct task_stat *task_stat_addr;

struct task_stat *get_task_stat(int tid)
{
	assert((tid >= 0) && (tid < OS_NR_TASKS));
	return &task_stat_addr[tid]; 
}

void task_stat_release(int tid)
{
	assert((tid >= 0) && (tid < OS_NR_TASKS));
	memset(&task_stat_addr[tid], 0, sizeof(struct task_stat));
}

void task_stat_update(struct task *task)
{
	struct task_stat *kstat = get_task_stat(task->tid);

	kstat->state = task->state;
	kstat->cpu = task->cpu;
	kstat->cpu_usage = 0x0;
	kstat->prio = task->prio;
}

void task_stat_init(struct task *task)
{
	struct task_stat *kstat;
	struct process *proc;

	if (task_stat_addr == NULL)
		return;

	kstat = get_task_stat(task->tid);
	kstat->tid = task->tid;
	kstat->pid = task->pid;
	kstat->start_ns = task->start_ns;
	kstat->state = task->state;
	kstat->cpu = task->cpu;
	kstat->prio = task->prio;
	kstat->cpu_usage = 0x0;

	if (!(task->flags & TASK_FLAGS_KERNEL)) {
		proc = task_to_proc(task);
		if (proc->root_task)
			kstat->root_tid = proc->root_task->tid;
	}
}

static void kernel_task_state_init(struct task *task)
{
	struct task_stat *ts;

	if (!(task->flags & TASK_FLAGS_KERNEL))
		return;

	ts = get_task_stat(task->tid);
	strcpy(ts->cmd, task->name);
	task_stat_init(task);
}


// FIXME: kobject
int procinfo_init(void)
{
	// struct pma_create_arg args;
	uint32_t memsz;
	// right_t right;
	// int ret;

	/*
	 * allocate pma kobject for process and task info which
	 * can shared to each process in these system.
	 */
	memsz = sizeof(struct task_stat) * OS_NR_TASKS;
	memsz = align_page_up(memsz);
	task_stat_addr = get_free_pages(memsz >> PAGE_SHIFT, GFP_USER);
	assert(task_stat_addr != NULL);
	// args.type = PMA_TYPE_PMEM;
	// args.right = KOBJ_RIGHT_RW;
	// args.consequent = 1;
	// args.start = vtop(task_stat_addr);
	// args.size = memsz;
	// ret = create_new_pma(&task_stat_pma, &right, &args);
	// ASSERT(ret == 0);
	memset(task_stat_addr, 0, memsz);
	printf("task stat memory size 0x%x\n", memsz);

	// register_hook(procinfo_switch_hook, OS_HOOK_TASK_SWITCH);

	/*
	 * init the kernel task's task stat.
	 */
	do_for_all_task(kernel_task_state_init);

	return 0;
}