#include <asm/arm64_common.h>
#include <cpu_feature.h>
#include <smp.h>



int affinity_to_cpuid(u64 affinity)
{
	int aff0, aff1;

	if (cpu_has_feature(CPU_FEATURE_MPIDR_SHIFT))  {
		aff0 = (affinity >> MPIDR_EL1_AFF1_SHIFT) & 0xff;
		aff1 = (affinity >> MPIDR_EL1_AFF2_SHIFT) & 0xff;
	} else {
		aff0 = (affinity >> MPIDR_EL1_AFF0_SHIFT) & 0xff;
		aff1 = (affinity >> MPIDR_EL1_AFF1_SHIFT) & 0xff;
	}

	return (aff1 * CONFIG_NR_CPUS_CLUSTER0) + aff0;
}


u64 cpuid_to_affinity(int cpuid)
{
	int aff0, aff1;

	if (cpu_has_feature(CPU_FEATURE_MPIDR_SHIFT))  {
		if (cpuid < CONFIG_NR_CPUS_CLUSTER0)
			return (cpuid << MPIDR_EL1_AFF1_SHIFT);
		else {
			aff0 = cpuid - CONFIG_NR_CPUS_CLUSTER0;
			aff1 = 1;

			return (aff1 << MPIDR_EL1_AFF2_SHIFT) |
				(aff0 << MPIDR_EL1_AFF1_SHIFT);
		}
	} else {
		if (cpuid < CONFIG_NR_CPUS_CLUSTER0) {
			return cpuid;
		} else {
			aff0 = cpuid - CONFIG_NR_CPUS_CLUSTER0;
			aff1 = 1;

			return (aff1 << MPIDR_EL1_AFF1_SHIFT) + aff0;
		}
	}
}