
#include <kernel.h>
#include <barrier.h>
#include <math.h>
#include <init.h>
#include <irq.h>
#include <timer.h>

#define CNT_CTL_ISTATUS (1 << 2)
#define CNT_CTL_IMASK   (1 << 1)
#define CNT_CTL_ENABLE  (1 << 0)

#define TIMER_PRECISION 1000000 // 1000000ns = 1ms
#define SECONDS(s)     		((uint64_t)((s)  * 1000000000ULL))
#define MILLISECS(ms)  		((uint64_t)((ms) * 1000000ULL))
#define MICROSECS(us)  		((uint64_t)((us) * 1000ULL))

#define tick2ns(tick) muldiv64(ticks, SECONDS(1), 1000*cpukhz)
#define ns2tick(ns)   muldiv64(ns, 1000*cpukhz, SECONDS(1))
#define systick() read_sysreg64(cntpct_el0)
#define systime() tick2ns(systick())

static u64 alarm_tick = 0;
static u64 cpukhz = 0;

static void 
timer_handler(int intid)
{
    printf("timer!!\n");
    timer_stop();
}

static int 
timer_irq_init(void)
{
  return irq_register(INTID_VTIMER, timer_handler, "timer");
}
irqhook_initcall(timer_irq_init);


void
timer_setup(u64 ns)
{
    if (ns < TIMER_PRECISION)
        ns = TIMER_PRECISION;
    alarm_tick = systick()+ns2tick(ns);
}

void 
timer_start(void)
{
  assert(alarm_tick != 0);

  u64 ctl = read_sysreg(CNTV_CTL_EL0);
  ctl |= CNT_CTL_ENABLE;
  ctl &= ~CNT_CTL_IMASK;
  write_sysreg(alarm_tick, CNTV_CVAL_EL0);
  write_sysreg(ctl, CNTV_CTL_EL0);
  isb();
}

void 
timer_stop(void)
{
  u64 ctl = read_sysreg(CNTV_CTL_EL0);
  ctl &= ~CNT_CTL_ENABLE;
  write_sysreg(ctl, CNTV_CTL_EL0);
  isb();
}


void 
init_timer(void)
{
    cpukhz = read_sysreg(CNTFRQ_EL0)/1000;
}