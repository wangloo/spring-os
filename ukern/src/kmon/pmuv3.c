#include <kernel.h>
#include <kmem.h>
#include <barrier.h>
#include <kmon/pmuv3.h>

/*
 * PMXEVTYPER: Event selection reg
 */
#define	ARMV8_PMU_EVTYPE_MASK	0xc800ffff	/* Mask for writable bits */
#define	ARMV8_PMU_EVTYPE_EVENT	0xffff		/* Mask for EVENT bits */

// This limit is architectural-defined.
// Real number usually less than it, A57 has 6 counters 
#define ARMV8_PMU_MAX_COUNTERS 32
#define ARMV8_PMU_COUNTER_MASK (ARMV8_PMU_MAX_COUNTERS - 1)

#define ARMV8_CYCLE_COUNTER ((u32)-1)


#define A57_PMU_MAX_COUNTERS 6
#define COUNTER_MAX_PERROID ((1llu << 32) - 1)

struct pmu_event {
#define PMU_COUTNER_CYCLE 0
#define PMU_COUTNER_PERF  1
  u32 counter_type;    
  u32 counter_idx;  
  u64 event;
};

struct pmu_event_filter {
  u32 exclude_user;
  u32 exclude_kernel;
  u32 exclude_hv;
};

// Abstract a PMU hardware
// Externally visible
struct pmu {
  char *name;

  void (*pmu_enable)(void);
  void (*pmu_disable)(void);

  void (*set_filter)(struct pmu_event *pe,struct pmu_event_filter *filter);
  
  // Install/Unstall a hw_event on PMU management
  // TODO: no other hw_event system now, not used now
  int (*install)(struct pmu_event *pe);
  int (*uninstall)(struct pmu_event *pe);

  // Start/Stop a counter present on the PMU
  void (*start_event)(struct pmu_event *pe);
  void (*stop_event)(struct pmu_event *pe);
};

////////////////////////

// Configure the event type binded to counter
void 
pmuv3_write_counter_event(int counter, unsigned long event)
{
  if (counter == ARMV8_CYCLE_COUNTER)
    write_pmccfiltr(event);
  else {
    event &= ARMV8_PMU_EVTYPE_MASK;
    write_pmevtypern(counter, event);
  }
}

// Value cause counter irq occurs
void 
pmuv3_write_counter_val(int counter, u64 val)
{
  if (counter == ARMV8_CYCLE_COUNTER)
    // write_pmccntr(val);
    return;
  else {
    val &= ARMV8_PMU_EVTYPE_MASK;
    write_pmevcntrn(counter, val);
  }
}
unsigned long
pmuv3_read_counter_val(int counter)
{
  if (counter == ARMV8_CYCLE_COUNTER)
    return read_pmccntr();
  else
    return read_pmevcntrn(counter);
}

void
pmuv3_enable_counter(int counter)
{
	/*
	 * Make sure event configuration register writes are visible before we
	 * enable the counter.
	 * */
	isb();
	write_pmcntenset(BIT(counter));
}

void
pmuv3_disable_counter(int counter)
{
  write_pmcntenclr(BIT(counter));
  /*
   * Make sure the effects of disabling the counter are visible before we
   * start configuring the event.
   */
  isb();
}

void
pmuv3_enable(void)
{
  if (((read_sysreg(ID_AA64DFR0_EL1) >> 8) & 0xF) < 1) {
    assert(0);
  }
  pmu_pmcr_write(pmu_pmcr_read() | ARMV8_PMU_PMCR_E);
}

void
pmuv3_disable(void)
{
  pmu_pmcr_write(pmu_pmcr_read() & ~ARMV8_PMU_PMCR_E);
}




#if 0
static int pmu_perf_counter;

struct pmu_event *
new_pmu_event(u32 counter_type, u64 event)
{
  struct pmu_event*pe;

  if ((counter_type == PMU_COUTNER_PERF) && 
      (pmu_perf_counter == A57_PMU_MAX_COUNTERS))
    return NULL;
        

  if (!(pe = kalloc(sizeof(*pe))))
    return NULL;

  if (counter_type == PMU_COUTNER_CYCLE)
    pe->counter_idx = 0;
  else
    pe->counter_idx = pmu_perf_counter++;
  pe->event = event;
  return pe;
}









static inline void
pmuv3_enable_counter_irq(u32 counter)
{
  write_pmintenset(BIT(counter));
}

void
pmuv3_disable_counter_irq(u32 counter)
{
  u32 mask = BIT(counter);
	write_pmintenclr(mask);
	isb();
	/* Clear the overflow flag in case an interrupt is pending. */
	write_pmovsclr(mask);
	isb();
}




static void
pmuv3_set_event_filter(struct pmu_event *pe, struct pmu_event_filter *filter)
{
  u64 event = pe->event & ARMV8_PMU_EVTYPE_EVENT;
  
  if (filter->exclude_kernel)
    event |= ARMV8_PMU_EXCLUDE_EL1;
  if (!filter->exclude_hv)
    event |= ARMV8_PMU_INCLUDE_EL2;
  if (filter->exclude_user)
    event |= ARMV8_PMU_EXCLUDE_EL0;

  pe->event = event;
}

static void
pmuv3_start_event(struct pmu_event *pe)
{
  u32 counter = pe->counter_idx;
  
  if (pe->counter_type == PMU_COUTNER_CYCLE)
    counter = -1;

  /*
   * For non-sampling runs, limit the sample_period to half
   * of the counter width. That way, the new counter value
   * is far less likely to overtake the previous one unless
   * you have some serious IRQ latency issues.
   */
  pmuv3_write_counter(counter, COUNTER_MAX_PERROID >> 1);

  pmuv3_disable_counter(counter);
  pmuv3_write_event_type(counter, pe->event);
  pmuv3_enable_counter_irq(counter);
  pmuv3_enable_counter(counter);
}

static void
pmuv3_stop_event(struct pmu_event *pe)
{
  u32 counter = pe->counter_idx;
  
  if (pe->counter_type == PMU_COUTNER_CYCLE)
    counter = -1;

  pmuv3_disable_counter(counter);
  pmuv3_disable_counter_irq(counter);
}



struct pmu curpmu = (struct pmu) {
  .pmu_enable = pmuv3_enable,
  .pmu_disable = pmuv3_disable,
  .set_filter = pmuv3_set_event_filter,
  .start_event = pmuv3_start_event,
  .stop_event = pmuv3_stop_event,

  .install = NULL,
  .uninstall = NULL,
};
#endif