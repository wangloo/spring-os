#include <arm64_sysreg.h>
#include <arm64_timer.h>
#include <time.h>
#include <barrier.h>
#include <kernel.h>


u32 cpu_khz;



unsigned long get_sys_tick(void)
{
  isb();
  return read_sysreg64(cntpct_el0);
}

unsigned long get_current_time(void)
{
  isb();
  assert(0);
  return 0;
  // return ticks_to_ns(read_sysreg64(cntpct_el0) - boot_tick);
}

unsigned long get_sys_time(void)
{
  isb();
  return ticks_to_ns(get_sys_tick());
}

void arch_timer_enable(void)
{
  unsigned long ctl;


  ctl = read_sysreg(ARM64_CNTSCHED_CTL);
  ctl |= CNT_CTL_ENABLE;
  ctl &= ~CNT_CTL_IMASK;
  write_sysreg(1, ARM64_CNTSCHED_CTL);
  isb();
}

void arch_timer_setddl(u64 ns)
{
  assert(ns > 0);
  write_sysreg(ns_to_ticks(ns), ARM64_CNTSCHED_CVAL);

}

void arch_timer_init()
{
  cpu_khz = read_sysreg32(CNTFRQ_EL0) / 1000;
}