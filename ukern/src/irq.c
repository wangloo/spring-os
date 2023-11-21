#include <kernel.h>
#include <smp.h>
#include <bitmap.h>
#include <cpumask.h>
#include <gic_v3.h>
#include <cpu.h>
#include <irq.h>

#define NR_PCPU_IRQ_DESC (NR_PCPU_IRQS * CONFIG_NR_CPUS)
#define NR_SPI_IRQ_DESC	 (NR_SPIS)

static void default_irq_handler(int irq)
{
	printf("irq %d is not register\n", irq);
}

static struct irq pcpu_irq_desc[NR_PCPU_IRQ_DESC] = {
	[0 ... (NR_PCPU_IRQ_DESC - 1)] = {
		.handler = default_irq_handler,
	},
};
static struct irq spi_irq_desc[NR_SPI_IRQ_DESC] = {
	[0 ... (NR_SPI_IRQ_DESC - 1)] = {
		 .handler = default_irq_handler,
	},
};

static inline struct irq *
irq_desc(u32 intid)
{
    int cpuid = cur_cpuid();
	if (intid <= PPI_MIN)
		return &pcpu_irq_desc[cpuid*NR_PCPU_IRQS + intid];
	return &spi_irq_desc[intid - PPI_MIN];
}




void irq_enable(u32 irq)
{
  gicv3_irq_enable(irq);
}

void irq_disable(u32 irq)
{
  gicv3_irq_disable(irq);
}

int 
irq_register(u32 intid, irq_handler_fp handler, char *name)
{
	struct irq *irq;

	if (!handler)
		return -EINVAL;

	if ((irq = irq_desc(intid)) == NULL)
		return -ENOENT;


	// spin_lock_irqsave(&irq_desc->lock, flag); // FIXME
	irq->intid = intid;
	irq->handler = handler;

	irq_enable(intid);
    LOG_DEBUG("IRQ", "here\n");
	// spin_unlock_irqrestore(&irq_desc->lock, flag); // FIXME

  return 0;
}


void send_sgi(u32 sgi, int cpuid)
{
  cpumask_t cpu;

	assert((cpuid >= 0) || (cpuid < CONFIG_NR_CPUS));

  cpumask_clearall(&cpu);
  cpumask_set_cpu(cpuid, &cpu);
  LOG_DEBUG("IRQ", "send sgi: %d to cpu %d\n", sgi, cpuid);
  gicv3_send_sgi(sgi, SGI_TO_LIST, &cpu);
}

u32 irq_read_pending(void)
{
	return gicv3_read_irq();
}

void irq_end(u32 irq)
{
	gicv3_eoi_irq(irq);
}

static void 
do_handle_host_irq(int cpuid, struct irq *irq)
{
	irq->handler(irq->intid);
}

// 中断handler
int do_irq_handler(void)
{
	struct irq *irq;
    int intid;
	int cpuid = cur_cpuid();

	while (1) {
		intid = irq_read_pending();

        // No more interrupt is pending
		if (intid >= IRQ_SPURIOUS)
			return 0;

        if ((irq = irq_desc(intid)) != NULL) {
            // printf("start handling irq: %d\n", intid);
            do_handle_host_irq(cpuid, irq);
        }
        irq_end(intid);
	}

	return 0;
}