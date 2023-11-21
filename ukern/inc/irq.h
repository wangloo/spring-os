#pragma once
#include <types.h>
#include <intid.h>
#include <spinlock.h>


typedef void (*irq_handler_fp)(int intid);

struct irq {
  struct spinlock lock;
  u16 intid;
  u16 affinity;
  irq_handler_fp handler;
};




#define irq_is_disable()        arch_irq_is_disable()

#define arch_irq_is_disable()   (read_sysreg(daif) & (1u << DAIF_I_SHIFT))

int irq_register(u32 irq, irq_handler_fp handler, char *name);

void send_sgi(u32 sgi, int cpuid);
u32 irq_read_pending(void);
void irq_end(u32 irq);
int do_irq_handler(void);