#include <kernel.h>
#include <intid.h>
#include <init.h>
#include <irq.h>
#include <arch_timer.h>
#include <time.h>

#define TIMER_PRECISION 1000000 // 1000000ns = 1ms

#define systick() arch_counter_get_cntpct()
#define systime() tick2ns(systick())

static u64 alarm_tick = 0;

unsigned long
ftrace_timer_tick(void)
{
  return systick();
}

void 
ftrace_timer_start(void)
{
  assert(alarm_tick != 0);

  u64 ctl = arch_timer_reg_read(ARCH_TIMER_REG_V_CTRL);
  ctl |= CNT_CTL_ENABLE;
  ctl &= ~CNT_CTL_IMASK;
  arch_timer_reg_write(ARCH_TIMER_REG_V_CVAL, alarm_tick);
  arch_timer_reg_write(ARCH_TIMER_REG_V_CTRL, ctl);
}

void 
ftrace_timer_stop(void)
{
  u64 ctl = arch_timer_reg_read(ARCH_TIMER_REG_V_CTRL);
  ctl &= ~CNT_CTL_ENABLE;
  arch_timer_reg_write(ARCH_TIMER_REG_V_CTRL, ctl);
}

static void 
ftrace_timer_handler(int intid)
{
    printf("timer!!\n");
    ftrace_timer_stop();
}

static int 
ftrace_timer_irq_init(void)
{
  return irq_register(INTID_PTIMER, ftrace_timer_handler, "sched-timer");
}
irqhook_initcall(ftrace_timer_irq_init);


void
ftrace_timer_setup(u64 ns)
{
    if (ns < TIMER_PRECISION)
        ns = TIMER_PRECISION;
    alarm_tick = systick()+ns2tick(ns);
}




int 
init_ftrace_timer(void)
{
  if (init_arch_timer() < 0)
    return -1;
  return 0;
}
