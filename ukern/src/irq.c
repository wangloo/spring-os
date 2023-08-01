#include <irq.h>
#include <smp.h>
#include <kernel.h>

static int default_irq_handler(uint32_t irq, void *data);

static struct irq_desc percpu_irq_descs[PERCPU_IRQ_DESC_SIZE] = {
	[0 ... (PERCPU_IRQ_DESC_SIZE - 1)] = {
		.handler = default_irq_handler,
	},
};

static struct irq_desc spi_irq_descs[SPI_IRQ_DESC_SIZE] = {
	[0 ... (SPI_IRQ_DESC_SIZE - 1)] = {
		 .handler = default_irq_handler,
	},
};

static inline struct irq_desc *get_irq_desc_cpu(int cpuid, u32 irq)
{
	if (irq >= NR_SPIS)
		return NULL;

	if (irq <= PPI_MIN)
		return &percpu_irq_descs[cpuid * NR_PERCPU_IRQS + irq];

	return &spi_irq_descs[irq - SPI_MIN];
}

/*
 * notice, when used this function to get the percpu
 * irqs need to lock the kernel to invoid the thread
 * sched out from this cpu and running on another cpu
 *
 * so usually, percpu irq will handle in kernel contex
 * and not in task context
 */
struct irq_desc *irq_get_desc(u32 irq)
{
	return get_irq_desc_cpu(cpu_id(), irq);
}


static int default_irq_handler(u32 irq, void *data)
{
	printf("irq %d is not register\n", irq);
	return 0;
}


void irq_enable(u32 irq)
{
  gicv3_irq_enable(irq);
}

void irq_disable(u32 irq)
{
  gicv3_irq_disable(irq);
}

void irq_set_type(u32 irq, int type)
{
  // TODO
  // 设置一个irq是电平触发 or 边沿触发
}


int irq_register(u32 irq, irq_handle_t handler, 
      unsigned long flags, char *name, void *data)
{
	int type;
	struct irq_desc *irq_desc;

	// unused(name);

	if (!handler)
		return -EINVAL;

	irq_desc = irq_get_desc(irq);
	if (!irq_desc)
		return -ENOENT;

	type = flags & IRQ_FLAGS_TYPE_MASK;
	flags &= ~IRQ_FLAGS_TYPE_MASK;

	// spin_lock_irqsave(&irq_desc->lock, flag); // FIXME
	irq_desc->handler = handler;
	irq_desc->pdata = data;
	irq_desc->flags |= flags;
	irq_desc->intid = irq;

	/* enable the hw irq and set the mask bit */
	irq_enable(irq);
	irq_desc->flags &= ~IRQ_FLAGS_MASKED;

	if (irq < SPI_MIN)
		irq_desc->affinity = cpu_id();

	// spin_unlock_irqrestore(&irq_desc->lock, flag); // FIXME

	if (type)
		irq_set_type(irq, type);
  return 0;
}


void send_sgi(u32 sgi, int cpuid)
{
  cpumask_t cpu;

	assert((cpuid >= 0) || (cpuid < CONFIG_NR_CPUS));

  cpumask_clearall(&cpu);
  cpumask_set_cpu(cpuid, &cpu);
  LOG_DEBUG("IRQ", "send sgi: %d to cpu %d", sgi, cpuid);
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

static int do_handle_host_irq(int cpuid, struct irq_desc *irq_desc)
{
	int ret;

	if (cpuid != irq_desc->affinity) {
		printf("irq %d do not belong to this cpu\n", irq_desc->intid);
		ret =  -EINVAL;
		goto out;
	}

	ret = irq_desc->handler(irq_desc->intid, irq_desc->pdata);

out:
	irq_end(irq_desc->intid);
	return ret;
}

// 中断handler
int do_irq_handler(void)
{
	uint32_t irq;
	struct irq_desc *irq_desc;
	int cpuid = cpu_id();

	while (1) {
		irq = irq_read_pending();
		if (irq >= IRQ_SPURIOUS)
			return 0;

		irq_desc = get_irq_desc_cpu(cpuid, irq);
		if (!irq_desc) {
			printf("irq is not actived %d\n", irq);
			irq_end(irq);
			continue;
		}
		printf("start handling irq: %d\n", irq);
		do_handle_host_irq(cpuid, irq_desc);
		printf("end handling irq: %d\n", irq);
	}

	return 0;
}