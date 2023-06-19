#include <tid.h>
#include <bitmap.h>
#include <task_def.h>

static bitmap_t tid_map[bits_to_long(OS_NR_TASKS)];
// static DEFINE_SPIN_LOCK(tid_lock);

int alloc_tid(void)
{
	int tid = -1;

	// spin_lock(&tid_lock);

	tid = bitmap_find_next_0(tid_map, OS_NR_TASKS, 1);
	if (tid >= OS_NR_TASKS)
		tid = -1;
	else
		bitmap_set_bit(tid_map, tid);

	// spin_unlock(&tid_lock);

	return tid;
}


void release_tid(int tid)
{
	assert((tid < OS_NR_TASKS) && (tid > 0));

    assert(0);
	// os_task_table[tid] = NULL; // FIXME: doit elsewhere
	smp_wmb();
	bitmap_clear_bit(tid_map, tid);
}

int tid_init(void)
{
	/*
	 * tid is reserved for system use.
	 */
	bitmap_set_bit(tid_map, 0);

	return 0;
}