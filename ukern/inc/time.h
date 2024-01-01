#pragma once
#include <math.h>

#define SECONDS(s)     		((unsigned long)((s)  * 1000000000ULL))
#define MILLISECS(ms)  		((unsigned long)((ms) * 1000000ULL))
#define MICROSECS(us)  		((unsigned long)((us) * 1000ULL))

extern unsigned long cpukhz;

static inline unsigned long
tick2ns(unsigned long ticks)
{
  return muldiv64(ticks, SECONDS(1), 1000*cpukhz);
}
static inline unsigned long 
ns2tick(unsigned long ns)   
{
  return muldiv64(ns, 1000*cpukhz, SECONDS(1));
}
