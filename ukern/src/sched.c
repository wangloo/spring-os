#include <pcpu.h>
#include <barrier.h>
#include <timer.h>
#include <time.h>
#include <current.h>
#include <irq.h>
#include <sched.h>
#include <preempt.h>
#include <current.h>
#include <bitops.h>
#include <kernel.h>

int8_t const ffs_one_table[256] = {
    -1, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x00 to 0x0F */
    4,  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x10 to 0x1F */
    5,  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x20 to 0x2F */
    4,  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x30 to 0x3F */
    6,  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x40 to 0x4F */
    4,  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x50 to 0x5F */
    5,  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x60 to 0x6F */
    4,  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x70 to 0x7F */
    7,  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x80 to 0x8F */
    4,  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x90 to 0x9F */
    5,  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0xA0 to 0xAF */
    4,  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0xB0 to 0xBF */
    6,  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0xC0 to 0xCF */
    4,  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0xD0 to 0xDF */
    5,  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0xE0 to 0xEF */
    4,  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0  /* 0xF0 to 0xFF */
};


static inline void sched_check(void)
{
  if (in_interrupt() || (irq_is_disable() && !preempt_allowed()))
    panic("sched is disabled %s %d\n", __func__, __LINE__);
}



static inline int sched_allowed(void)
{
  return preempt_allowed() && !irq_is_disable();
}


static void sched_tick_handler(unsigned long data)
{
  struct task *task = current();

  /*
   * mark this task has used its running ticket, and the sched
   * timer is off.
   */
  task->ti.flags |= __TIF_TICK_EXHAUST;
  set_need_resched();
}


void sched_update_sched_timer(void)
{
  struct pcpu *pcpu = get_pcpu(cpu_id());
  struct task *task = current();

  /*
   * enable the sched timer if there are more than one
   * ready task on the same prio.
   */
  if ((pcpu->tasks_in_prio[task->prio] > 1))
    timer_setup_and_start(&pcpu->sched_timer, MILLISECS(task->run_time));
  else
    timer_stop(&pcpu->sched_timer);
}




static struct task *pick_next_task(struct pcpu *pcpu)
{
  struct list_head *head;
  struct task *task = current();
  int prio;

  /*
   * if the current task need to sleep or waitting some
   * event happen. delete it from the ready list, then the
   * next run task can be got.
   */
  mb();
  assert(task->state != TASK_STATE_READY);

  if (!task_is_running(task)) {
    remove_task_from_ready_list(pcpu, task);
    if (task->state == TASK_STATE_STOP) {
      list_add_tail(&task->state_list, &pcpu->stop_list);
      // FIXME
      // flag_set(&pcpu->kworker_flag, KWORKER_TASK_RECYCLE);
    }
  }

  /*
   * get the highest ready task list to running
   */
  prio = ffs_one_table[pcpu->local_rdy_grp];
  assert(prio != -1);
  head = &pcpu->ready_list[prio];

  /*
   * get the first task, then put the next running
   * task to the end of the ready list.
   */
  assert(!list_empty(head));
  task = list_first_entry(head, struct task, state_list);
  list_del(&task->state_list);
  list_add_tail(&task->state_list, head);

  return task;
}


int select_task_run_cpu(void)
{
  // TODO
  return 0;
  // return NR_CPUS - 1;
}

static void switch_to_task(struct task *cur, struct task *next)
{
  struct pcpu *pcpu = get_pcpu(cpu_id());

  // arch_task_sched_out(cur); // FIXME
  // do_hooks(cur, NULL, OS_HOOK_TASK_SWITCH_OUT); //FIXME

  /*
   * check the current task's state and do some action
   * to it, check whether it suspend time is set or not
   *
   * if the task is ready state, adjust the run time of
   * this task. If the task need to wait some event, and
   * need request a timeout timer then need setup the timer.
   */
  if ((cur->state == TASK_STATE_WAIT_EVENT) && (cur->delay > 0))
    timer_setup_and_start(&cur->delay_timer, MILLISECS(cur->delay));
  else if (cur->state == TASK_STATE_RUNNING)
    cur->state = TASK_STATE_READY;

  cur->last_cpu = cur->cpu;
  cur->run_time = TASK_RUN_TIME;
  smp_wmb();

  /*
   * notify the cpu which need to waku-up this task that
   * the task has been do to sched out, can be wakeed up
   * safe, the task is offline now.
   */
  cur->cpu = -1;
  smp_wmb();

  /*
   * change the current task to next task.
   */
  next->state = TASK_STATE_RUNNING;
  next->ti.flags &= ~__TIF_TICK_EXHAUST;
  next->cpu = pcpu->pcpu_id;
  set_current_task(next);
  pcpu->running_task = next;

  next->ctx_sw_cnt++;
  next->wait_event = 0;
  next->start_ns = get_sys_time();
  smp_wmb();

  // do_hooks(next, NULL, OS_HOOK_TASK_SWITCH_TO); // FIXME
  arch_task_sched_in(next);
}


static inline int __sched_prepare(void)
{
  struct task *next, *task = current();
  struct task_info *ti = get_task_info(task);
  struct pcpu *pcpu = get_pcpu(cpu_id());

  /*
   * if the task is suspend state, means next the cpu
   * will call sched directly, so do not sched out here
   *
   * 1 - when preempt_count > 0, the scheduler whill try
   *     to shced() when preempt_enable.
   * 2 - __TIF_DONOT_PREEMPT is set, it will call sched() at
   *    once.
   */
  if (!(ti->flags & __TIF_NEED_RESCHED) || (ti->preempt_count > 0)
      || (ti->flags & __TIF_DONOT_PREEMPT))
    goto task_run_again;

  ti->flags &= ~__TIF_NEED_RESCHED;

  next = pick_next_task(pcpu);
  printf("cur task: %s ==> next task: %s\n", task->name, next->name);
  if ((next == task)) goto task_run_again;

  switch_to_task(task, next);
  // do_hooks(task, next, OS_HOOK_TASK_SWITCH); // FIXME

  return 0;

task_run_again:
  if (test_and_clear_bit(TIF_TICK_EXHAUST, &ti->flags))
    return -EAGAIN;
  else
    return -EACCES;
}
void sched_prepare(void)
{
  int ret = __sched_prepare();

  if ((ret == 0) || (ret == -EAGAIN)) 
    sched_update_sched_timer();
}


static int irqwork_handler(uint32_t irq, void *data)
{
	struct pcpu *pcpu = get_pcpu(cpu_id());
	struct task *task, *n;
	int preempt = 0, need_preempt;

	/*
	 * check whether there are new taskes need to
	 * set to ready state again
	 */
	// raw_spin_lock(&pcpu->lock);
  
	list_for_each_entry_safe(task, n, &pcpu->new_list, state_list) {
    printf("=============\n");
		/*
		 * remove it from the new_next.
		 */
		list_del(&task->state_list);

		if (task->state == TASK_STATE_RUNNING) {
			printf("task %s state %d wrong\n",
				task->name? task->name : "Null", task->state);
			continue;
		}

		need_preempt = task_need_resched(task);
		preempt += need_preempt;
		task_clear_resched(task);

    printf("handling task: %s\n", task->name);
		add_task_to_ready_list(pcpu, task, need_preempt);
		task->state = TASK_STATE_READY;

		/*
		 * if the task has delay timer, cancel it.
		 */
		if (task->delay) {
			timer_stop(&task->delay_timer);
			task->delay = 0;
		}
	}
	// raw_spin_unlock(&pcpu->lock);

	if (preempt || task_is_idle(current()))
		set_need_resched();

	return 0;
}

static int wake_up_interrupted(struct task *task,
		long pend_state, unsigned long data)
{
	// unsigned long flags;

	assert(pend_state != TASK_STATE_PEND_TO);
	if (task->state != TASK_STATE_WAIT_EVENT)
		return -EACCES;

	if (!irq_is_disable())
		panic("unexpected irq happend when wait_event() ?\n");

	/*
	 * the interrup occurs when task try to wait_event. in
	 * addition:
	 * 1 - the interrupt is happended in the same cpu.
	 * 2 - will not the delay timer, since the delay time
	 *     has not been set already.
	 * 3 - the state must TASK_STATE_WAIT_EVENT
	 * 4 - task has not been in sched routine.
	 *
	 * meanwhile, other cpu may already in the wake up function
	 * try to wake up the task, then need check this suitation
	 * since other cpu while check cpu == -1, this will lead
	 * to dead lock if use spin_lock function. So here use
	 * spin_trylock instead.
	 */
  // FIXME
	// if (!spin_trylock_irqsave(&task->s_lock, flags))
	// 	return -EBUSY;

	// if (task->state != TASK_STATE_WAIT_EVENT) {
	// 	spin_unlock_irqrestore(&task->s_lock, flags);
	// 	return -EINVAL;
	// }

	task->ti.flags |= __TIF_WAIT_INTERRUPTED;
	task->ti.flags &= ~__TIF_DONOT_PREEMPT;

	/*
	 * here this cpu got this task, and can set the new
	 * state to running and run it again.
	 */
	task->pend_state = pend_state;
	task->state = TASK_STATE_RUNNING;
	task->delay = 0;
	task->ipcdata = data;
	// spin_unlock_irqrestore(&task->s_lock, flags);

	return 0;
}

static int wake_up_common(struct task *task, long pend_state, unsigned long data)
{
	// unsigned long flags;
	uint32_t timeout;

	preempt_disable();
	// spin_lock_irqsave(&task->s_lock, flags); // FIXME

	/*
	 * task already waked up, if the stat is set to
	 * TASK_STATE_WAIT_EVENT, it means that the task will
	 * call sched() to sleep or wait something happen.
	 */
	if (task->state != TASK_STATE_WAIT_EVENT) {
		// spin_unlock_irqrestore(&task->s_lock, flags); // FIXME
		preempt_enable();
		return -EPERM;
	}

	/*
	 * the task may in sched() routine on other cpu
	 * wait the task really out of running. since the task
	 * will not preempt in the kernel space now, so the cpu
	 * of the task will change to -1 at one time.
	 *
	 * since the kernel can not be preempted so it can make
	 * sure that sched() can be finish its work.
	 */
	while (task->cpu != -1)
		cpu_relax();

	/*
	 * here this cpu got this task, and can set the new
	 * state to running and run it again.
	 */
	task->pend_state = pend_state;
	task->state = TASK_STATE_WAKING;
	timeout = task->delay;
	task->delay = 0;
	task->ipcdata = data;

	// spin_unlock_irqrestore(&task->s_lock, flags);

	/*
	 * here it means that this task has not been timeout, so can
	 * delete the timer for this task.
	 */
	if (timeout && (task->pend_state != TASK_STATE_PEND_TO))
		timer_stop(&task->delay_timer);

	/*
	 * find a best cpu to run this task.
	 */
	task_ready(task, 1);
	preempt_enable();


	return 0;
}

int __wake_up(struct task *task, long pend_state, unsigned long data)
{

	if (task == current())
		return wake_up_interrupted(task, pend_state, data);
	else
		return wake_up_common(task, pend_state, data);
}



int sched_init(void)
{
  int i;


  for (i = 0; i < NR_CPUS; i++) pcpu_sched_init(get_pcpu(i));
  printf("sched: init ok\n");
  return 0;
}

void sched_local_init(void)
{
  int cpuid = cpu_id();
  struct pcpu *pcpu = get_pcpu(cpuid);

  // current task is store in the x18
  // x18 will store the current task pointer
  set_current_task(task_idle_pcpu(cpuid));

  timer_init(&pcpu->sched_timer, sched_tick_handler, (unsigned long)pcpu);
	irq_register(CONFIG_KERNEL_IRQWORK_IRQ, irqwork_handler,
			0, "irqwork handler", NULL);
  printf("sched: local init ok\n");
}


void cond_resched(void)
{
  if (need_resched() && sched_allowed()) sched();
}

static void inline sys_sched(void)
{
  sched_check();
  arch_sys_sched();
}

void sched(void)
{
  /*
   * tell the scheduler that I am ok to sched out.
   */
  set_need_resched();
  // clear_do_not_preempt();

  do {
    sys_sched();
  } while (need_resched());
}
