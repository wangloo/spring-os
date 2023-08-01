/**
 * @file kernel.h
 * @author your name (you@domain.com)
 * @brief  包括一些内核中常用的头文件，许多地方就不用每次去include它们了。
 *         另外还包括一些内核才能使用的通用宏/函数, 如ptov()
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once


#include <config/config.h>
#include <assert.h>
#include <string.h>
#include <print.h>
#include <errno.h>
#include <utils.h>
#include <qemu.h>
#include <log.h>
#include <compiler.h>

#define ptov(addr)	((unsigned long)addr + CONFIG_KERNEL_ADDR_MASK)
#define vtop(addr)	((unsigned long)addr - CONFIG_KERNEL_ADDR_MASK)
#define __va(va)	((unsigned long)(va))
#define is_kva(va)	((unsigned long)va >= CONFIG_KERNEL_ADDR_MASK)


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