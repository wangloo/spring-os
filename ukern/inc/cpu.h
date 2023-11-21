#pragma once
#include <list.h>
#include <spinlock.h>

/* cpu 全局状态控制 */
#define cpu_irq_on()        asm("msr daifclr, #2" : : : "memory")
#define cpu_irq_off()       asm("msr daifset, #2" : : : "memory")
#define cpu_fiq_on()        asm("msr daifclr, #1" : : : "memory")
#define cpu_fiq_off()       asm("msr daifset, #1" : : : "memory")
#define cpu_async_on()      asm("msr daifclr, #4" : : : "memory")
#define cpu_async_off()     asm("msr daifset, #4" : : : "memory")
#define cpu_debug_on()      asm("msr daifclr, #8" : : : "memory")
#define cpu_debug_off()     asm("msr daifset, #8" : : : "memory")

#define cpu_intr_off()   do {\
    cpu_irq_off();           \
    cpu_fiq_off();           \
} while(0)

#define cpu_intr_on()   do {\
    cpu_irq_on();           \
    cpu_fiq_on();           \
} while(0)

static inline void cpu_relax(void)
{
  asm volatile("yield" ::: "memory");
}

// Describe a cpu's state
struct cpu {
  int id;              // Fixed place, don't change
  struct proc *proc;   // The process running on this cpu, or null.

  // The followings are about mutil-level scheduling
  // based on priority
  uint8_t local_rdy_grp;
  struct list_head ready_list[PROC_PRIO_MAX];
  
  void *intr_stack;      // interrupt stack base 
  struct spinlock lock;
}__cache_line_align;

struct cpu*
cur_cpu(void);
int 
cur_cpuid(void);
void
init_cpus(void);