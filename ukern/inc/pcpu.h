#pragma once
#include <task_def.h>
#include <smp.h>

typedef enum {
  PCPU_STATE_OFFLINE = 0x0,
  PCPU_STATE_RUNNING,
  PCPU_STATE_IDLE,
} pcpu_state_t;

/* 抽象一个CPU */
struct pcpu {
  int pcpu_id; // fixed place, do not change.
  volatile int state;

  void *stack;

  unsigned long percpu_offset;

  /*
   * each pcpu has its local sched list, 8 priority
   * local_rdy_grp only use [0 - 8], in these 8
   * priority:
   * 7 - used for idle task
   * 6 - used for vcpu task
   *
   * only the new_list can be changed by other cpu, the
   * lock is for the new_list.
   */
  // spinlock_t lock;
  struct list_head new_list;
  struct list_head die_process;

  struct list_head stop_list;
  struct task *running_task; // 该CPU上正在运行的task
  struct task *idle_task;    // 该CPU的idle
  uint32_t nr_pcpu_task;

  /* 以下成员为优先级位图 */
  uint8_t local_rdy_grp;
  uint8_t padding[3];
  struct list_head ready_list[OS_PRIO_MAX];
  int tasks_in_prio[OS_PRIO_MAX];


  struct timer sched_timer;
  int os_is_running;

	struct task *kworker;
	// struct flag_grp kworker_flag; // TODO

} __cache_line_align;





struct pcpu *get_pcpu(int cpuid);
void add_task_to_ready_list(struct pcpu *pcpu, struct task *task,
                                   int preempt);
void remove_task_from_ready_list(struct pcpu *pcpu, struct task *task);
void pcpu_task_ready(struct pcpu *pcpu, struct task *task, int preempt);
void pcpu_smp_task_ready(struct pcpu *pcpu,
		struct task *task, int preempt);
void percpu_init(void);
void pcpu_sched_init(struct pcpu *pcpu);
void pcpu_irqwork(int pcpu_id);


// TODO: 这两个函数的作用是什么？ 是否可以删除？或者用
//       pcpu->state来代替判断？
static inline int os_is_running(void)
{
	return get_pcpu(cpu_id())->os_is_running;
}

static inline void set_os_running(void)
{
	/* 
	 * os running is set before the irq is enable
	 * so do not need to aquire lock or disable the
	 * interrupt here
	 */
	get_pcpu(cpu_id())->os_is_running = 1;
	wmb();
}
