#include <task.h>
#include <sched.h>
#include <irq.h>
#include <cpu.h>
#include <preempt.h>
#include <pcpu.h>
#include <tid.h>
#include <kmem.h>
#include <event.h>
#include <mm.h>
#include <kernel.h>


/*
 * set current running task's state do not need to obtain
 * a lock, when need to wakeup the task, below state the state
 * can be changed:
 * 1 - running -> wait_event
 * 2 - wait_event -> running (waked up by event)
 * 3 - new -> running
 * 4 - running -> stopped
 */
#define set_current_state(_state, to)                                          \
  do {                                                                         \
    current()->state = (_state);                                               \
    current()->delay = (to);                                                   \
    smp_mb();                                                                  \
  } while (0)


struct task *os_task_table[OS_NR_TASKS];

/* idle task needed be static defined */
struct task idle_tasks[NR_CPUS];

struct task *task_idle_pcpu(int cpuid)
{
  assert(cpuid < NR_CPUS);
  return &idle_tasks[cpuid];
}


// 将task放入合适的ready list
int task_ready(struct task *task, int preempt)
{
  struct pcpu *pcpu, *tpcpu;

  preempt_disable();

  task->cpu = task->affinity;
  if (task->cpu == -1) task->cpu = select_task_run_cpu();

  /*
   * if the task is a precpu task and the cpu is not
   * the cpu which this task affinity to then put this
   * cpu to the new_list of the pcpu and send a resched
   * interrupt to the pcpu
   */
  pcpu = get_pcpu(cpu_id());
  if (pcpu->pcpu_id != task->cpu) {
    tpcpu = get_pcpu(task->cpu);
    pcpu_smp_task_ready(tpcpu, task, preempt);
  } else {
    pcpu_task_ready(pcpu, task, preempt);
  }

  preempt_enable();
  return 0;
}

static int __wake_up_interrupted(struct task *task, long pend_state, unsigned long data)
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

static int __wake_up_common(struct task *task, long pend_state, unsigned long data)
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


int task_wakeup(struct task *task, long pend_state, unsigned long data)
{
  if (task == current())
    return __wake_up_interrupted(task, pend_state, data);
  else
    return __wake_up_common(task, pend_state, data);
}


void task_sleep(uint32_t delay)
{
  TODO();
  // FIXME
  // struct task *task = current();
  // unsigned long flags;

  // /*
  //  * task sleep will wait for the sleep timer expired
  //  * or the event happend
  //  */
  // local_irq_save(flags);
  // do_not_preempt();
  // task->delay = (delay == (uint32_t)-1 ? TASK_WAIT_FOREVER : delay);
  // task->state = TASK_STATE_WAIT_EVENT;
  // task->wait_type = OS_EVENT_TYPE_TIMER;
  // task->wait_event = NULL;
  // local_irq_restore(flags);

  // sched();
}

static inline void __task_stop(int state)
{
  TODO();
  // FIXME
  // struct task *task = current();
  // unsigned long flags;

  // do_not_preempt();
  // local_irq_save(flags);
  // task->delay = 0;
  // task->state = state;
  // task->wait_type = 0;
  // local_irq_restore(flags);

  // sched();
}

void task_suspend(void)
{
  __task_stop(TASK_STATE_SUSPEND);
}

void task_die(void)
{
  __task_stop(TASK_STATE_STOP);
}

void task_exit(int errno)
{
  set_current_state(TASK_STATE_STOP, 0);
  sched();
}

void task_return_to_user(gp_regs *regs)
{
  struct task *task = current();
  unsigned long flags = task->ti.flags;

  assert(!(task->flags & TASK_FLAGS_KERNEL));
  task->ti.flags &= ~(flags | (__TIF_NEED_STOP | __TIF_NEED_FREEZE));
  smp_wmb();

  if (flags & __TIF_NEED_STOP)
    task->state = TASK_STATE_STOP;
  else if (flags & __TIF_NEED_FREEZE)
    task->state = TASK_STATE_SUSPEND;

  if (task->state != TASK_STATE_RUNNING) {
    sched();
    panic("%s %d: should not be here\n", __func__, __LINE__);
  }

  // if (task->return_to_user)
  // 	task->return_to_user(task, regs);
}

static void task_delaytimer_timeout_handler(unsigned long data)
{
  // FIXME
  // struct task *task = (struct task *)data;

  // wake_up_timeout(task);
  // set_need_resched();
}



void do_for_all_task(void (*hdl)(struct task *task))
{
  struct task *task;
  int idx;

  // get the tid_lock ?
  bitmap_for_each_set_bit(idx, tid_map, BITMAP_SIZE(OS_NR_TASKS)) {
    task = os_task_table[idx];
    if (!task) continue;
    hdl(task);
  }
}

static void task_init(struct task *task, char *name, void *stack,
                      uint32_t stk_size, int prio, int tid, int aff,
                      unsigned long opt, void *arg)
{
  /*
   * idle task is setup by create_idle task, skip
   * to setup the stack information of idle task, by
   * default the kernel stack will set to stack top.
   */
  if (!(opt & TASK_FLAGS_IDLE)) {
    task->stack_bottom = stack;
    task->stack_top = stack + stk_size;
    task->stack_base = task->stack_top;

    TASK_INFO_INIT(&task->ti, task);
  }

  task->tid = tid;
  task->prio = prio;
  task->pend_state = 0;
  task->flags = opt;
  task->pdata = arg;
  task->affinity = aff;
  task->run_time = TASK_RUN_TIME;
  // spin_lock_init(&task->s_lock);
  task->state = TASK_STATE_SUSPEND;
  task->cpu = -1;


  timer_init(&task->delay_timer, task_delaytimer_timeout_handler,
             (unsigned long)task);


  os_task_table[tid] = task;
  if (name)
    strncpy(task->name, name, min(strlen(name), TASK_NAME_SIZE));
  else
    sprintf(task->name, "task%d", tid);
}

static void task_create_hook(struct task *task, void *pdata)
{
  // FIXME
  // do_hooks((void *)task, pdata, OS_HOOK_CREATE_TASK);
}

static struct task *do_create_task(char *name,
                  task_func_t func,
                  uint32_t ssize,
                  int prio,
                  int tid,
                  int aff,
                  unsigned long opt,
                  void *arg)
{
  size_t stk_size = align_page_up(ssize);
  struct task *task;
  void *stack = NULL;

  /*
   * allocate the task's kernel stack
   */
  task = kalloc(sizeof(struct task));
  if (!task) {
    printf("no more memory for task\n");
    return NULL;
  }

  stack = get_free_pages(stk_size / PAGE_SIZE, GFP_KERNEL);
  if (!stack) {
    printf("no more memory for task stack\n");
    kfree(task);
    return NULL;
  }

  task_init(task, name, stack, stk_size, prio, tid, aff, opt, arg);
  return task;
}

struct task *__create_task(char *name,
            task_func_t func,
            uint32_t stk_size,
            void *usp,
            int prio,
            int aff,
            unsigned long opt,
            void *arg)
{
  struct task *task;
  int tid;

  if ((aff >= NR_CPUS) && (aff != TASK_AFF_ANY)) {
    printf("task %s afinity will set to 0x%x\n", name, TASK_AFF_ANY);
    aff = TASK_AFF_ANY;
  }

  if ((prio >= OS_PRIO_IDLE) || (prio < 0)) {
    printf("wrong task prio %d fallback to %d\n", prio, OS_PRIO_DEFAULT_6);
    prio = OS_PRIO_DEFAULT_6;
  }

  tid = alloc_tid();
  if (tid < 0) 
    return NULL;

  preempt_disable();

  task = do_create_task(name, func, stk_size, prio, tid, aff, opt, arg);
  if (!task) {
    release_tid(tid);
    preempt_enable();
    return NULL;
  }

  // task_create_hook(task, arg);
  // TODO: `  暂时先做成这样， 后期也是做成hook的形式
  extern int process_task_create_hook(void *item, void *context);
  process_task_create_hook(task, arg);

  arch_init_task(task, (void *)func, usp, task->pdata);

  /*
   * start the task if need auto started.
   */
  if (!(task->flags & TASK_FLAGS_NO_AUTO_START)) 
    task_ready(task, 0);

  preempt_enable();

  if (os_is_running()) 
    sched();

  return task;
}

struct task *create_task(char *name,
        task_func_t func,
        size_t stk_size,
        void *usp,
        int prio,
        int aff,
        unsigned long opt,
        void *arg)
{
  if (prio < 0) {
    if (opt & OS_PRIO_VCPU)
      prio = OS_PRIO_VCPU;
    else if (opt & (TASK_FLAGS_SRV | TASK_FLAGS_DRV))
      prio = OS_PRIO_SRV;
    else
      prio = OS_PRIO_DEFAULT;
  }

  return __create_task(name, func, stk_size, usp, prio, aff, opt, arg);
}

void *__get_kernel_stack_top(void); // FIXME: 放到合适的位置
int create_idle_task(void)
{
  struct task *task;
  char task_name[32];
  int cpuid = cpu_id();
  int tid = OS_NR_TASKS - 1 - cpuid;
  struct pcpu *pcpu = get_pcpu(cpu_id());

  task = &idle_tasks[cpuid];
  // assert(!request_tid(tid));  // FIXME

  sprintf(task_name, "idle/%d", cpuid);

  task_init(task, task_name, NULL, 0, OS_PRIO_IDLE, tid, cpuid,
            TASK_FLAGS_IDLE | TASK_FLAGS_KERNEL, NULL);
  
  // TODO: 和 kernel 共用一个栈好吗？
  task->stack_top =
      __get_kernel_stack_top() - (cpuid << CONFIG_TASK_STACK_SHIFT);
  task->stack_bottom = task->stack_top - TASK_STACK_SIZE;
  task->state = TASK_STATE_RUNNING;
  task->cpu = cpuid;
  task->run_time = 0;


  pcpu->running_task = task;
  set_current_task(task);
  /* call the hooks for the idle task */
  task_create_hook(task, NULL);

  list_add_tail(&task->state_list, &pcpu->ready_list[task->prio]);
  pcpu->local_rdy_grp |= BIT(task->prio);
  pcpu->idle_task = task;

  return 0;
}

struct task *create_kthread(char *name, task_func_t func, int prio,
                    int aff, unsigned long opt, void *arg)
{
    return create_task(name, func, TASK_STACK_SIZE, NULL, 
                prio, aff, opt | TASK_FLAGS_KERNEL, arg);
}

