#pragma once
#include <types.h>


enum cpu_feature_bit {
    CPU_FEATURE_VHE = 0,
    CPU_FEATURE_MPIDR_SHIFT,

    CPU_FEATURE_BITS
};

static int cpu_has_feature(u64 feat)
{
    if (feat >= CPU_FEATURE_BITS)
        return 0;
    
    // TODO: 每个bit代表不同的feature
    return 0;   
} 