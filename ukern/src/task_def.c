// #include <task.h>

#if 0
#include <timer.h>
#include <task_info.h>
#include <vspace.h>
#include <ctx.h>
#include <list.h>
#include <task_config.h>


typedef int (*task_func_t)(void *data);


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
  struct list_head proc_list;  // task的所有process
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

  struct cpu_context cpu_context;
};

#endif