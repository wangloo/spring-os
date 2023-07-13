#pragma once
#include <config/config.h>

#define ptov(addr)	((unsigned long)addr + CONFIG_KERNEL_ADDR_MASK)
#define vtop(addr)	((unsigned long)addr - CONFIG_KERNEL_ADDR_MASK)
#define __va(va)	((unsigned long)(va))
#define is_kva(va)	((unsigned long)va >= CONFIG_KERNEL_ADDR_MASK)