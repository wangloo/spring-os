#pragma once
#include <config/config.h>
#include <size.h>
#include <assert.h>
#include <string.h>
#include <print.h>
#include <errno.h>
#include <types.h>
#include <ctype.h>
#include <utils.h>
#include <qemu.h>
#include <log.h>
#include <compiler.h>
#include <utils_arm64.h>

#define ptov(addr)	((unsigned long)(addr) + CONFIG_KERNEL_ADDR_MASK)
#define vtop(addr)	((unsigned long)(addr) - CONFIG_KERNEL_ADDR_MASK)
#define __va(va)	((unsigned long)(va))
#define is_kva(va)	((unsigned long)(va) >= CONFIG_KERNEL_ADDR_MASK)


#define SPR_OK   0
#define SPR_ERR  1

static inline void exit() 
{
    qemu_exit();
}

static void panic(const char *str, ...)
{
  va_list ap;

  puts("\n");
  
  va_start(ap, str);
  vprintf(str, ap);
  va_end(ap);

  exit();
}