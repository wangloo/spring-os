
#include <arm64_timer.h>
#include <preempt.h>
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
  if (timer->timeout < TIMER_PRECISION) timer->timeout = TIMER_PRECISION;
}

void timer_setup_and_start(struct timer *timer, u64 ns)
{
  timer->timeout = ns;
  timer_start(timer);
}

int timer_stop(struct timer *timer)
{
  // struct raw_timer *timers = timer->raw_timer;
  // unsigned long flags;

  // if (timer->cpu == -1)
  // 	return 0;

  // timer->stop = 1;
  // ASSERT(timer->raw_timer != NULL);
  // spin_lock_irqsave(&timers->lock, flags);
  // /*
  //  * wait the timer finish the action if already
  //  * timedout.
  //  */
  // while (timers->running_timer == timer)
  // 	cpu_relax();

  // detach_timer(timers, timer);
  // timer->cpu = -1;
  // timer->expires = 0;
  // spin_unlock_irqrestore(&timers->lock, flags);

  return 0;
}

void timer_init(struct timer *timer, timer_func_t fn, unsigned long data)
{
  // arch_timer_init();

  preempt_disable();

  // timer->cpu = -1;
  // timer->entry.next = NULL;
  timer->expires = 0;
  timer->timeout = 0;
  timer->hook = fn;
  timer->data = data;
  // timer->raw_timer = NULL;
  preempt_enable();
}
