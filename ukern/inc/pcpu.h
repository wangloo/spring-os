#pragma once
#include <task_def.h>


typedef enum {
  PCPU_STATE_OFFLINE = 0x0,
  PCPU_STATE_RUNNING,
  PCPU_STATE_IDLE,
} pcpu_state_t;


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

  uint8_t local_rdy_grp;
  uint8_t padding[3];
  struct list_head ready_list[OS_PRIO_MAX];
  int tasks_in_prio[OS_PRIO_MAX];


  struct timer sched_timer;


} __cache_line_align;




struct pcpu *get_pcpu(int cpuid);
void percpu_init(void);
void pcpu_sched_init(struct pcpu *pcpu);