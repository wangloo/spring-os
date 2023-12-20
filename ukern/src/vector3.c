
#include <kernel.h>
#include <spinlock.h>
#include <list.h>
#include <proc.h>
#include <cpu.h>
#include <esr.h>
#include <irq.h>
#include <syscall.h>
#include <kmon/kmon.h>


void 
sync_from_current_el(struct econtext *ectx)
{
  // struct econtext *ectx;
  int ec;

  // Interrupt is disable here
  // So use cur_proc() will enable interrupt improperly
  // ectx = proc_ectx(cur_cpu()->proc);
  ec = ectx->esr.ec;
  
  printf("SYNC FROM CURRENT EL\n");
  printf("EC: 0x%lx\n", ec);
  printf("      \"%s\"\n", get_ec_string(ec));
  printf("ELR: %lx\n", ectx->ctx.elr);
  printf("Sp : %lx\n", ectx->ctx.sp);
  printf("FAR: %lx\n", ectx->far);


  backtrace(ectx->ctx.elr, ectx->ctx.sp);
  // kmon_main();
  panic("SPRING-OS oops!\n");
}


void 
sync_from_lower_el(void)
{
  struct econtext *ectx;
  int ec;

  // Interrupt is disable here
  // So use cur_proc() will enable interrupt improperly
  ectx = proc_ectx(cur_cpu()->proc);
  ec = ectx->esr.ec;

  if (ec == ESR_ELx_EC_SVC64) {
    syscall_handler(&ectx->ctx.gp_regs);
    return;
  }

  printf("SYNC FROM LOWER EL\n");
  printf("ESR: 0x%lx\n", ectx->esr);
  printf("- EC: 0x%lx\n", ec);
  printf("      \"%s\"\n", get_ec_string(ec));
  printf("ELR: 0x%lx\n", ectx->ctx.elr);
  printf("FAR: 0x%lx\n", ectx->far);

  panic("SPRING-OS oops!\n");
}



void 
irq_from_current_el(void)
{
  printf("IRQ FROM CURRENT EL\n");
  do_irq_handler();
  // panic("SPRING-OS oops!\n");
}


void 
irq_from_lower_el(void)
{
  printf("IRQ FROM LOWER EL\n");
  panic("SPRING-OS oops!\n");
}
