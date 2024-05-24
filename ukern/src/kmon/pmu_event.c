#include <kernel.h>
#include <barrier.h>
#include <kmon/pmuv3.h>
void
pmu_event_enable(int counter, unsigned long event)
{
  pmuv3_write_counter_event(counter, event);
  // LOG_DEBUG("write_pmevtypern: %d\n", read_pmevtypern(counter));
  pmuv3_write_counter_val(counter, 0);
  pmuv3_enable_counter(counter);

}


void
pmu_event_disable(int counter)
{
  pmuv3_disable_counter(counter);
}

void
pmu_event_print(int counter)
{
  printf("Counter %d's value: %ld\n", 
            counter, pmuv3_read_counter_val(counter));
}