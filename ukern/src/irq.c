#include <gic_v3.h>



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

}

