#pragma once
#include <timer.h>
#include <task_info.h>
#include <ctx.h>
#include <list.h>
#include <smp.h>
#include <config/config.h>

#define NR_CPUS                   CONFIG_NR_CPUS
#define OS_NR_TASKS               CONFIG_NR_TASKS

#define OS_PRIO_MAX               8
#define OS_PRIO_DEFAULT_0         0
#define OS_PRIO_DEFAULT_1         1
#define OS_PRIO_DEFAULT_2         2
#define OS_PRIO_DEFAULT_3         3
#define OS_PRIO_DEFAULT_4         4
#define OS_PRIO_DEFAULT_5         5
#define OS_PRIO_DEFAULT_6         6
#define OS_PRIO_DEFAULT_7         7

#define OS_PRIO_REALTIME          OS_PRIO_DEFAULT_0
#define OS_PRIO_SRV               OS_PRIO_DEFAULT_2
#define OS_PRIO_SYSTEM            OS_PRIO_DEFAULT_3
#define OS_PRIO_VCPU              OS_PRIO_DEFAULT_4
#define OS_PRIO_DEFAULT           OS_PRIO_DEFAULT_5
#define OS_PRIO_IDLE              OS_PRIO_DEFAULT_7
#define OS_PRIO_LOWEST            OS_PRIO_IDLE

#define TASK_FLAGS_SRV            BIT(0) // should not change, need keep same as pangu
#define TASK_FLAGS_DRV            BIT(1)
#define TASK_FLAGS_VCPU           BIT(2)
#define TASK_FLAGS_REALTIME       BIT(3)
#define TASK_FLAGS_IDLE           BIT(4)
#define TASK_FLAGS_NO_AUTO_START  BIT(5)
#define TASK_FLAGS_32BIT          BIT(6)
#define TASK_FLAGS_PERCPU         BIT(7)
#define TASK_FLAGS_DEDICATED_HEAP BIT(8)
#define TASK_FLAGS_ROOT           BIT(9)
#define TASK_FLAGS_KERNEL         BIT(10)

#define TASK_STATE_RUNNING        0x00
#define TASK_STATE_READY          0x01
#define TASK_STATE_WAIT_EVENT     0x02
#define TASK_STATE_WAKING         0x04
#define TASK_STATE_SUSPEND        0x08
#define TASK_STATE_STOP           0x10

#ifdef CONFIG_TASK_RUN_TIME
#define TASK_RUN_TIME CONFIG_TASK_RUN_TIME
#else
#define TASK_RUN_TIME 50
#endif

#ifdef CONFIG_TASK_STACK_SIZE
#define TASK_STACK_SIZE CONFIG_TASK_STACK_SIZE
#else
#define TASK_STACK_SIZE (2 * PAGE_SIZE)
#endif

#define TASK_NAME_SIZE    (32)

#define TASK_WAIT_FOREVER (0xfffffffe)

struct task {
  struct task_info ti;

  void *stack_base;
  void *stack_top;
  void *stack_bottom;

  gp_regs *user_regs;

  unsigned long flags;

  int pid;
  int tid;

  struct list_head list;
  struct list_head proc_list;
  struct list_head task_list;  // link to the task list, if is a thread.
  struct list_head state_list; // link to the sched list used for sched.

  uint32_t delay;
  struct timer delay_timer;

  /*
   * the spinlock will use to protect the below member
   * which may modified by different cpu at the same
   * time:
   * 1 - state
   * 2 - pend_state
   */
  // spinlock_t s_lock;
  int state;
  long pend_state;

  int wait_type;               // which event is task waitting for.
  void *wait_event;            // the event instance which the task is waitting.
  struct list_head event_list;
  struct flag_node *flag_node; // used for the flag event.
  union {
    unsigned long ipcdata;
    long retcode;
    void *msg;
    long flags_rdy;
  };


  /*
   * affinity - the cpu node which the task affinity to
   */
  int cpu;
  int last_cpu;
  int affinity;
  int prio;

  unsigned long run_time;   // 时间片长度 ms

  unsigned long ctx_sw_cnt; // switch count of this task.
  unsigned long start_ns;   // when the task started last time.

  char name[TASK_NAME_SIZE];

  struct vspace *vs; // the virtual memory space of this task.

  void *pdata;       // the private data of this task for vcpu or process.
};
