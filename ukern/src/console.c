#include <kernel.h>
#include <uart_pl011.h>
#include <irq.h>
#include <init.h>
#include <kmon/kmon.h>
#include <console.h>

struct dev_console g_cons = {
  .name = "pl011",
  .init = uart_init,
  .irq_init = uart_irq_init,
  .getc = uart_getc,
  .putc = uart_putc,
};

static void
console_irq_handler(int intid)
{
    char c = g_cons.getc();
    if (c == 0x03) {
      kmon_main();
    }
}

static int
console_irq_init(void)
{
  g_cons.irq_init();
  return irq_register(INTID_UART_NS, console_irq_handler, "pl011-recv");
}
irqhook_initcall(console_irq_init);



void 
init_console()
{
  initlock(&g_cons.lock, "cons");
  g_cons.init();
}

void console_putc(char ch)
{
  g_cons.putc(ch);
}

char 
console_getc(void)
{
  return g_cons.getc();
}

int console_puts(char *buf, size_t size)
{
	size_t print = 0;

	while (print < size)
		console_putc(buf[print++]);

	return size;
}
