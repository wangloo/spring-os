#include <exception.h>
#include <esr.h>
#include <panic.h>
#include <uspace/syscall.h>


void sync_from_current_el(exception_ctx *ectx)
{
  int ec = ectx->esr.ec;

  printf("SYNC FROM CURRENT EL\n");
  printf("- EC: 0x%lx\n", ec);
  printf("      \"%s\"\n", get_ec_string(ec));
  printf("- ELR: 0x%lx\n", ectx->gregs.elr);
  panic("SPRING-OS oops!\n");

}


void sync_from_lower_el(exception_ctx *ectx)
{
  int ec = ectx->esr.ec;

  if (ec == ESR_ELx_EC_SVC64) {
    syscall_handler((syscall_regs *)&(ectx->gregs));
    return;
  }

  printf("SYNC FROM LOWER EL\n");
  printf("- EC: 0x%lx\n", ec);
  printf("      \"%s\"\n", get_ec_string(ec));
  printf("- ELR: 0x%lx\n", ectx->gregs.elr);
  panic("SPRING-OS oops!\n");

}
