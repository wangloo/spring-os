#include <exception.h>
#include <esr.h>
#include <panic.h>


void sync_from_current_el(exception_ctx *ectx)
{
  int ec = ectx->esr.ec;

  printf("SYNC FROM CURRENT EL\n");
  printf("- EC: 0x%lx\n", ec);
  printf("      \"%s\"\n", get_ec_string(ec));
  printf("- ELR: 0x%lx\n", ectx->gregs.elr);
}


void sync_from_lower_el(exception_ctx *ectx)
{
  printf("SYNC FROM LOWER EL\n");
  panic("SPRING-OS oops!\n");
}
