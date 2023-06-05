#include <arm64_sysreg.h>
#include <arm64_timer.h>
#include <time.h>
#include <barrier.h>
#include <assert.h>

#define CNT_CTL_ISTATUS		(1 << 2)
#define CNT_CTL_IMASK		(1 << 1)
#define CNT_CTL_ENABLE		(1 << 0)

u32 cpu_khz;

void arch_timer_enable(void)
{
	unsigned long ctl;


	ctl = read_sysreg(cntp_ctl_el0);
	ctl |= CNT_CTL_ENABLE;
	ctl &= ~CNT_CTL_IMASK;
	write_sysreg(ctl, cntp_ctl_el0);
	isb();
}

void arch_timer_setalarm(u32 ns)
{
  assert(ns > 0);
  write_sysreg(ns_to_ticks(ns), cntp_tval_el0);
}

unsigned long get_sys_tick(void)
{
	isb();
	return read_sysreg64(cntpct_el0);
}

unsigned long get_current_time(void)
{
	isb();
	assert(0);
	// return ticks_to_ns(read_sysreg64(cntpct_el0) - boot_tick);
}

unsigned long get_sys_time(void)
{
	isb();
	return ticks_to_ns(get_sys_tick());
}
