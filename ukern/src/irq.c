#include <gic_v3.h>
#include <assert.h>


void irq_enable(u32 irq)
{
  gicv3_irq_enable(irq);
}

void irq_disable(u32 irq)
{
  gicv3_irq_disable(irq);
}




void send_sgi(u32 sgi, int cpuid)
{
  cpumak_t cpu;

	assert((cpuid >= 0) || (cpuid < CONFIG_NR_CPUS));

  cpumask_clearall(&cpu);
  cpumask_set_cpu(cpuid, &cpu);

  gicv3_send_sgi(sgi, SGI_TO_LIST, &cpu);
}

