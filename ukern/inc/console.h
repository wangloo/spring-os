#pragma once
#include <types.h>

// 未来可以扩展多个 console
struct console {
  char *name;
  int (*init)(vaddr_t base_addr);
  void (*putc)(char ch);
  char (*getc)(void);
};


void console_init();
void console_putc(char ch);
char console_getc(void);
int console_puts(char *buf, size_t size);