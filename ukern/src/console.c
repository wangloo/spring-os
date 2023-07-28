#include <console.h>
#include <uart_pl011.h>
#include <kernel.h>
#include <spinlock.h>


struct console global_console = {
  .name = "pl011",
  .init = pl011_init,
  .getc = pl011_getc,
  .putc = pl011_putc,
};
struct console *cons = &global_console;
static DEFINE_SPIN_LOCK(cons_lock); // TODO: 集成到console结构体中

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

int console_puts(char *buf, size_t size)
{
	size_t print = 0;
	unsigned long flags;

	spin_lock_irqsave(&cons_lock, flags);

	while (print < size)
		console_putc(buf[print++]);

	spin_unlock_irqrestore(&cons_lock, flags);

	return size;
}