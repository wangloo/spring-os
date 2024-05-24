#include <kernel.h>
#include <list.h>
#include <spinlock.h>
#include <cpu.h>
#include <proc.h>
#include <sched2.h>

int8_t const ffs_one_table[256] = {
	-1, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x00 to 0x0F */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x10 to 0x1F */
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x20 to 0x2F */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x30 to 0x3F */
	6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x40 to 0x4F */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x50 to 0x5F */
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x60 to 0x6F */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x70 to 0x7F */
	7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x80 to 0x8F */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x90 to 0x9F */
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0xA0 to 0xAF */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0xB0 to 0xBF */
	6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0xC0 to 0xCF */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0xD0 to 0xDF */
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0xE0 to 0xEF */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0  /* 0xF0 to 0xFF */
};



static inline void *
asm_get_current_proc(void)
{
	void *p;
	__asm__ volatile ("mov %0, x18" : "=r" (p));
	return p;
}

static inline void 
asm_set_current_proc(void *p)
{
	__asm__ volatile ("mov x18, %0" : : "r" (p));
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  struct proc *p = cur_proc();
  acquire(&p->lock);
  p->state = RUNNABLE;
  sched();
  release(&p->lock);

  // FIXME: put p to ready list
}

static void 
switch_to(struct proc *cur, struct proc *next)
{
  // TODO: There need to be a reg(x18?) stores current process
  // So that exception return know what to restore
  asm_set_current_proc(next);
  proc_ectx(next)->ctx.gp_regs.x18 = (unsigned long)next;
}



// Per-CPU process scheduler.
// Each CPU calls scheduler_run() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run.
//  - swtch to start running that process.
//  - eventually that process transfers control
//    via swtch back to the scheduler.
void
scheduler_run(void)
{


}


void 
add_proc_to_ready_list(struct cpu *cpu, struct proc *proc)
{
  assert(proc->sched_list.next == NULL);
  printf("[DEBUG] add task: %s to ready list\n", proc->name);

  list_add_tail(&proc->sched_list, &cpu->ready_list[proc->prio]);
  cpu->local_rdy_grp |= bit(proc->prio);
}

void 
rm_proc_from_ready_list(struct cpu *cpu, struct proc *proc)
{
	assert(proc->sched_list.next != NULL);

	list_del(&proc->sched_list);
	if (list_empty(&cpu->ready_list[proc->prio]))
		cpu->local_rdy_grp &= ~bit(proc->prio);
}

static struct proc*
pick_next(void)
{
  struct list_head *head;
  struct cpu *cpu = cur_cpu();
  struct proc *np;
  int prio;

  // `prio` stores the highest priority of non-null ready list
  // `np` stores the front process in this ready list
  prio = ffs_one_table[cpu->local_rdy_grp];
  assert(prio != -1);
  head = &cpu->ready_list[prio];
  assert(!list_empty(head));
  np = list_first_entry(head, struct proc, sched_list);
//   LOG_DEBUG("hello\n");
  LOG_DEBUG("task %s ==> task %s\n", cur_proc()->name, np->name);

  // Move selected process(first) to last of the ready list
  // For fair
  list_del(&np->sched_list);
  list_add_tail(&np->sched_list, head);

  return np;
}


// Systick handler call this to schedule process
void
sched(void)
{
    struct proc *next;

    next = pick_next();

    // This assert is for debug, try to think if all processes
    // in the ready list is RUNNIABLE?
    assert(next->state == RUNNABLE);
    next->state = RUNNING;

    switch_to(cur_proc(), next);
}

