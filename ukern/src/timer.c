
#include <arm64_timer.h>
#include <timer.h>
#include <time.h>

void timer_start(struct timer *timer)
{
  timer->expires = get_sys_time() + timer->timeout;

  arch_timer_setddl(timer->expires);
  arch_timer_enable();
  
}

void timer_setup(struct timer *timer, u64 ns)
{
	timer->timeout = ns;
  if (timer->timeout < TIMER_PRECISION)
		timer->timeout = TIMER_PRECISION;
}

void timer_setup_and_start(struct timer *timer, u64 ns)
{
  timer->timeout = ns;
  timer_start(timer);
}
  
void timer_init(void)
{
  arch_timer_init();
}
