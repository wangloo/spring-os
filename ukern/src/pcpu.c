#include <pcpu.h>
#include <sched.h>
#include <arm64_pcpu.h>
#include <kmem.h>
#include <irq.h>
#include <task.h>

struct pcpu pcpus[NR_CPUS];

struct pcpu *get_pcpu(int cpuid)
{
  assert(cpuid < NR_CPUS);
  return &pcpus[cpuid];
}

static void set_percpu_pcpu(struct pcpu *pcpu)
{
  arch_set_percpu_pcpu(pcpu);
}

void add_task_to_ready_list(struct pcpu *pcpu, struct task *task,
                                   int preempt)
{
  /*
   * make sure the new task is insert to front of the current
   * task.
   *
   * if the prio is equal to the current task's prio, insert to
   * the front of the current task.
   */
  assert(task->state_list.next == NULL);
  pcpu->tasks_in_prio[task->prio]++;

  printf("[DEBUG] add task: %s to ready list\n", task->name);
  if (current()->prio == task->prio) {
    list_add_tail(&task->state_list, &current()->state_list);
    if (pcpu->tasks_in_prio[task->prio] == 2) 
      sched_update_sched_timer();
  } else {
    list_add_tail(&task->state_list, &pcpu->ready_list[task->prio]);
  }

  mb();
  pcpu->local_rdy_grp |= BIT(task->prio);

  if (preempt || current()->prio > task->prio) set_need_resched();
}

void remove_task_from_ready_list(struct pcpu *pcpu, struct task *task)
{
  assert(task->state_list.next != NULL);

  list_del(&task->state_list);
  if (list_empty(&pcpu->ready_list[task->prio]))
    pcpu->local_rdy_grp &= ~BIT(task->prio);
  mb();

  pcpu->tasks_in_prio[task->prio]--;

  /*
   * check whether need to stop the sched timer.
   */
  if ((current()->prio == task->prio) && (pcpu->tasks_in_prio[task->prio] == 1))
    sched_update_sched_timer();
}

void pcpu_task_ready(struct pcpu *pcpu, struct task *task, int preempt)
{
	// unsigned long flags;

	// local_irq_save(flags); // FIXME
	add_task_to_ready_list(pcpu, task, preempt);
	// local_irq_restore(flags);
}

void pcpu_smp_task_ready(struct pcpu *pcpu,
		struct task *task, int preempt)
{
	// unsigned long flags;

	if (preempt)
		task_set_resched(task);

	assert(task->state_list.next == NULL);
	// spin_lock_irqsave(&pcpu->lock, flags); // FIXME
	list_add_tail(&task->state_list, &pcpu->new_list);
	// spin_unlock_irqrestore(&pcpu->lock, flags); // FIXME

  // 往另一个CPU添加了一个任务， 故发送核间中断通知它
  // 重新调度，因为该任务可能优先级较高
	pcpu_irqwork(pcpu->pcpu_id);
}

void pcpu_sched_init(struct pcpu *pcpu)
{
  INIT_LIST_HEAD(&pcpu->new_list);
  INIT_LIST_HEAD(&pcpu->stop_list);
  INIT_LIST_HEAD(&pcpu->die_process);
  INIT_LIST_HEAD(&pcpu->ready_list[0]);
  INIT_LIST_HEAD(&pcpu->ready_list[1]);
  INIT_LIST_HEAD(&pcpu->ready_list[2]);
  INIT_LIST_HEAD(&pcpu->ready_list[3]);
  INIT_LIST_HEAD(&pcpu->ready_list[4]);
  INIT_LIST_HEAD(&pcpu->ready_list[5]);
  INIT_LIST_HEAD(&pcpu->ready_list[6]);
  INIT_LIST_HEAD(&pcpu->ready_list[7]);
}

void pcpu_irqwork(int pcpu_id)
{
	send_sgi(CONFIG_KERNEL_IRQWORK_IRQ, pcpu_id);
}

void percpu_init(void)
{
  struct pcpu *pcpu = get_pcpu(cpu_id());
  int pages = TASK_STACK_SIZE >> PAGE_SHIFT;

  // put pcpu in TPIDR
  set_percpu_pcpu(pcpu);

  // set pcpu stack
  pcpu->stack = get_free_pages(pages, GFP_KERNEL);
  assert(pcpu->stack);
  pcpu->stack += TASK_STACK_SIZE;
  pcpu->state = PCPU_STATE_RUNNING;
  
  printf("pcpu: init ok\n");
}