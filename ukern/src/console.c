#include <console.h>
#include <uart_pl011.h>
#include <kernel.h>

struct dev_console g_cons = {
  .name = "uart",
  .init = uart_init,
  .getc = uart_getc,
  .putc = uart_putc,
};


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


void 
console_handle(char c)
{
  console_putc(c);
}



