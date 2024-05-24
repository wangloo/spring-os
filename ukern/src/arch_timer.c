#include <kernel.h>
#include <compiler.h>
#include <barrier.h>
#include <arch_timer.h>

unsigned long cpukhz = 0;

int 
init_arch_timer(void)
{
  cpukhz = arch_timer_get_cntfrq() / 1000;
  return 0;
}
