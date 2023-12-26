#include <kernel.h>
#include <memlayout.h>
#include <uart_pl011.h>

#define DEFAULT_CLOCK (24000000) // clock frequency: 24M
#define BAUDRATE      (115200)

static struct uart_pl011 uart;

static void 
calculate_set_divisiors(const struct uart_pl011 *u)
{
  // 64 * F_UARTCLK / (16 * B) = 4 * F_UARTCLK / B
  const u32 div = 4 * u->base_clock / u->baudrate;
  u32 fractional, integer;

  fractional = div & 0x3f;
  integer = (div >> 6) & 0xffff;

  u->regs->FBRD = fractional;
  u->regs->IBRD = integer;
}

static inline void 
wait_tx_complete(volatile struct uart_pl011_regs *regs)
{
  while ((regs->FR & FR_BUSY) != 0) {}
}



// Asynchronous recv
char 
uart_getc(void)
{
  while (uart.regs->FR & FR_RXFE);
  return (char)(uart.regs->DR);
}

void uart_putc(char c)
{
  uart.regs->DR = c;
  wait_tx_complete(uart.regs);
}

void
uart_irq_init(void)
{
  struct uart_pl011 *u = &uart;

  /* Disable UART before anything else */
  u->regs->CR &= ~CR_UARTEN;

  /* Clear out any spuriously appearing RX interrupts */
  u->regs->ICR = ICR_RTIC | ICR_RXIC | ICR_TXIC;

  /* Enable Receive timeout interrupt and Receive interrupt */
  u->regs->IMSC = IMSC_RTIM | IMSC_RXIM;

  /* Finally enable UART */
  u->regs->CR |= CR_UARTEN;
}

int 
uart_init(void)
{
  struct uart_pl011 *u = &uart;
  u32 lcr;

  u->base_clock = DEFAULT_CLOCK;
  u->baudrate = BAUDRATE;
  u->data_bits = 8;
  u->stop_bits = 1;
  u->regs = (volatile struct uart_pl011_regs *)(ptov(UART_PL011_BASE));

  // Set frequency divisors (UARTIBRD and UARTFBRD) to configure the speed
  calculate_set_divisiors(u);

  // Disable UART before anything else
  u->regs->CR &= ~(CR_UARTEN);
  wait_tx_complete(u->regs);

  /* Configure FIFO */
  lcr = 0x0;
  // WLEN part of UARTLCR_H, you can check that this calculation does the
  // right thing for yourself
  lcr |= ((u->data_bits - 1) & 0x3) << 5;
  // Configure the number of stop bits
  if (u->stop_bits == 2)
      lcr |= LCR_STP2;

  // Enable FIFO
  //lcr |= LCR_FEN;
  //u->regs->LCR_H = lcr;

  // Disalbe all interrupts by setting corresponding bits to 0
  u->regs->IMSC = 0x0;
  // Disable DMA by setting all bits to 0
  u->regs->DMACR = 0x0;

  // Enable uart TX and RX
  u->regs->CR |= CR_TXEN | CR_RXEN;
  // Finally enable UART
  u->regs->CR |= CR_UARTEN;

  return 0;
}