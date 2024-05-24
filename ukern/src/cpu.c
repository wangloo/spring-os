#include <kernel.h>
#include <list.h>
#include <page.h>
#include <cpu.h>


static struct cpu cpus[NCPU];


static inline void 
asm_set_current_cpu(void *cpu)
{
	asm volatile ("msr tpidr_el1, %0" : : "r" (cpu));
}

int 
cur_cpuid(void)
{
	int cpu;
#ifdef CONFIG_SMP_OK
	uint64_t v;
	__asm__ volatile (
		"mrs	%0, TPIDR_EL1\n"
		"ldrh	%w1, [%0, #0]\n"
		: "=r" (v), "=r" (cpu)
		:
		: "memory"
	);
#else
    cpu = 0;
#endif
	return cpu;
}


// Return this CPU's cpu struct.
// Interrupts must be disabled.
struct cpu*
cur_cpu(void) 
{
  int id = cur_cpuid();

  struct cpu *c = &cpus[id];
  return c;
}


void
init_cpus(void)
{
  struct cpu *cpu;

  for (int i = 0; i < NCPU; i++) {
    cpu = &cpus[i];
    cpu->id = i;
    cpu->proc = NULL;
    cpu->intr_stack = page_allocn(2);
    cpu->local_rdy_grp = 0;
    initlock(&cpu->lock, "cpu");
    for (int j = 0; j < PROC_PRIO_MAX; j++) {
      INIT_LIST_HEAD(&cpu->ready_list[j]);
    }

    // LOG_DEBUG("cpu%d intr stack base: 0x%lx\n", i, cpu->intr_stack);
    // LOG_DEBUG("addr of intr_stack: %p\n", &(cpu->intr_stack));
    // LOG_DEBUG("offset: %d\n", offsetof(struct cpu, intr_stack));
  }   

  // FIXME
  asm_set_current_cpu(&cpus[0]);
}