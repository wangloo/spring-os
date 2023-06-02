#pragma once
#include <print.h>
#include <console.h>

static inline void exit_qemu()
{
  asm volatile(
      /* 0x20026 == ADP_Stopped_ApplicationExit */
      "mov x1, #0x26\n"
      "movk x1, #2, lsl #16\n"
      "str x1, [sp,#0]\n"

      /* Exit status code. Host QEMU process exits with that status. */
      "mov x0, #0\n"
      "str x0, [sp,#8]\n"

      /* x1 contains the address of parameter block.
       * Any memory address could be used. */
      "mov x1, sp\n"

      /* SYS_EXIT */
      "mov w0, #0x18\n"

      /* Do the semihosting call on A64. */
      "hlt 0xf000\n");
}

static void panic(const char *str, ...)
{
  char buf[128];
  va_list ap;
  int count;
  char *cur = buf;

  va_start(ap, str);
  count = vsnprintf(buf, sizeof(buf), str, ap);
  va_end(ap);

  while (count > 0) {
    console_putc(*cur);
    ++cur, --count;
  }
  exit_qemu();
}