#pragma once
#include <asm/arm64_common.h>
#include <arm64_sysreg.h>
#include <gic_v3.h>

#define PERCPU_IRQ_DESC_SIZE	(NR_PERCPU_IRQS * CONFIG_NR_CPUS)
#define SPI_IRQ_DESC_SIZE	    (NR_SPIS)

#define IRQ_FLAGS_NONE           		(0x00000000)
#define IRQ_FLAGS_EDGE_RISING    		(0x00000001)
#define IRQ_FLAGS_EDGE_FALLING  		(0x00000002)
#define IRQ_FLAGS_LEVEL_HIGH     		(0x00000004)
#define IRQ_FLAGS_LEVEL_LOW      		(0x00000008)
#define IRQ_FLAGS_SENSE_MASK     		(0x0000000f)
#define IRQ_FLAGS_INVALID        		(0x00000010)
#define IRQ_FLAGS_EDGE_BOTH \
    (IRQ_FLAGS_EDGE_FALLING | IRQ_FLAGS_EDGE_RISING)
#define IRQ_FLAGS_LEVEL_BOTH \
    (IRQ_FLAGS_LEVEL_LOW | IRQ_FLAGS_LEVEL_HIGH)
#define IRQ_FLAGS_TYPE_MASK			(0x000000ff)

#define IRQ_FLAGS_MASKED_BIT			(8)
#define IRQ_FLAGS_MASKED			(BIT(IRQ_FLAGS_MASKED_BIT))
#define IRQ_FLAGS_PERCPU_BIT			(9)
#define IRQ_FLAGS_PERCPU			(BIT(IRQ_FLAGS_PERCPU_BIT))

#define IRQ_FLAGS_VCPU_BIT			(10)
#define IRQ_FLAGS_VCPU				(BIT(IRQ_FLAGS_VCPU_BIT))
#define IRQ_FLAGS_USER_BIT			(11)
#define IRQ_FLAGS_USER				(BIT(IRQ_FLAGS_USER_BIT))
#define IRQ_FLAGS_RECEIVER_MASK			(IRQ_FLAGS_USER | IRQ_FLAGS_VCPU)

#define IRQ_FLAGS_PENDING_BIT			(12)


typedef int (*irq_handle_t)(uint32_t irq, void *data);

struct irq_desc {
  irq_handle_t handler;
  unsigned long flags;
  u16 intid;
  u16 affinity;
  void *pdata;

};


/* Interrupt Control */
#define pstate_irq_enable()    asm("msr	daifclr, #2" : : : "memory")
#define pstate_irq_disable()   asm("msr	daifset, #2" : : : "memory")
#define pstate_fiq_enable()    asm("msr	daifclr, #1" : : : "memory")
#define pstate_fiq_disable()   asm("msr	daifset, #1" : : : "memory")
#define pstate_async_enable()  asm("msr	daifclr, #4" : : : "memory")
#define pstate_async_disable() asm("msr	daifset, #4" : : : "memory")
#define pstate_debug_enable()  asm("msr	daifclr, #8" : : : "memory")
#define pstate_debug_disable() asm("msr	daifset, #8" : : : "memory")

#define irq_is_disable()        arch_irq_is_disable()

#define arch_irq_is_disable()   (read_sysreg(daif) & (1u << DAIF_I_SHIFT))

int irq_register(u32 irq, irq_handle_t handler, 
      unsigned long flags, char *name, void *data);
void send_sgi(u32 sgi, int cpuid);
u32 irq_read_pending(void);
void irq_end(u32 irq);
int do_irq_handler(void);