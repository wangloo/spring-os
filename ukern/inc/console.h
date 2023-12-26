#pragma once
#include <types.h>
#include <spinlock.h>

// 未来可以扩展多个 console
struct dev_console {
  char *name;
  struct spinlock lock;

  int (*init)();
  void (*irq_init)();
  void (*putc)(char ch);
  char (*getc)(void);
};


void 
init_console();
void console_putc(char ch);
char console_getc(void);
int console_puts(char *buf, size_t size);