#pragma once

#include <types.h>
#include <cpu_feature.h>

int affinity_to_cpuid(u64 affinity);
u64 cpuid_to_affinity(int cpuid);