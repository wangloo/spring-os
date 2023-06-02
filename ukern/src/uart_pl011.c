#include <uart_pl011.h>

#define NO_POLL_CHAR  (-1)
static struct pl011 muart;

static void calculate_set_divisiors(const struct pl011 *dev)
{
  // 64 * F_UARTCLK / (16 * B) = 4 * F_UARTCLK / B
  const u32 div = 4 * dev->base_clock / dev->baudrate;
  u32 fractional, integer;

  fractional = div & 0x3f;
  integer = (div >> 6) & 0xffff;

  dev->regs->FBRD = fractional;
  dev->regs->IBRD = integer;
}

static inline void wait_tx_complete(const volatile struct pl011_regs *regs)
{
  while ((regs->FR & FR_BUSY) != 0) {}
}

int pl011_init(vaddr_t base_addr)
{
  struct pl011 *dev = &muart;
  u32 lcr;

  dev->base_clock = DEFAULT_CLOCK;
  dev->baudrate = BAUDRATE;
  dev->data_bits = 8;
  dev->stop_bits = 1;
  dev->regs = (struct pl011_regs *)base_addr;

  // Set frequency divisors (UARTIBRD and UARTFBRD) to configure the speed
  calculate_set_divisiors(dev);

  // Disable UART before anything else
  dev->regs->CR &= ~(CR_UARTEN);
  wait_tx_complete(dev->regs);

  /* Configure FIFO */
  lcr = 0x0;
  // WLEN part of UARTLCR_H, you can check that this calculation does the
  // right thing for yourself
  lcr |= ((dev->data_bits - 1) & 0x3) << 5;
  // Configure the number of stop bits
  if (dev->stop_bits == 2)
      lcr |= LCR_STP2;

  // Enable FIFO
  lcr |= LCR_FEN;
  dev->regs->LCR_H = lcr;

  // Disalbe all interrupts by setting corresponding bits to 0
  dev->regs->IMSC = 0x0;
  // Disable DMA by setting all bits to 0
  dev->regs->DMACR = 0x0;

  // Enable uart TX and RX
  dev->regs->CR |= CR_TXEN | CR_RXEN;
  // Finally enable UART
  dev->regs->CR |= CR_UARTEN;
  return 0;
}

char pl011_getc(void)
{
  while (muart.regs->FR & FR_RXFE);
  return (char)(muart.regs->DR);
}

void pl011_putc(char c)
{
  muart.regs->DR = c;
  wait_tx_complete(muart.regs);
}
