#include <exception.h>
#include <esr.h>
#include <kernel.h>
#include <irq.h>

void irq_from_current_el(exception_ctx *ectx)
{
  printf("IRQ FROM CURRENT EL\n");
  do_irq_handler();
  // panic("SPRING-OS oops!\n");
}


void irq_from_lower_el(exception_ctx *ectx)
{
  printf("IRQ FROM LOWER EL\n");
  panic("SPRING-OS oops!\n");
}
