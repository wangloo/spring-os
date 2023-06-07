#include <exception.h>
#include <esr.h>
#include <panic.h>

void irq_from_current_el(exception_ctx *ectx)
{
  printf("IRQ FROM CURRENT EL\n");
  panic("SPRING-OS oops!\n");
}


void irq_from_lower_el(exception_ctx *ectx)
{
  printf("IRQ FROM LOWER EL\n");
  panic("SPRING-OS oops!\n");
}
