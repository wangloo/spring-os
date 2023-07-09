#include <console.h>
#include <uart_pl011.h>
#include <config/config.h>
#include <addrspace.h>

struct console global_console = {
  .name = "pl011",
  .init = pl011_init,
  .getc = pl011_getc,
  .putc = pl011_putc,
};
struct console *cons = &global_console;

void console_init()
{
  if (cons->init)
    cons->init(ptov(CONFIG_UART_BASE));
}

void console_putc(char ch)
{
  cons->putc(ch);
}

char console_getc(void)
{
  return cons->getc();
}
