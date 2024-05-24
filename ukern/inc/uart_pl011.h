#pragma once
#include <types.h>



#define FR_BUSY       (1u << 3)
#define FR_RXFE       (1u << 4)
#define CR_RXEN       (1u << 9)
#define CR_TXEN       (1u << 8)
#define CR_UARTEN     (1u << 0)
#define IMSC_RTIM     (1u << 6)
#define IMSC_TXIM     (1u << 5)
#define IMSC_RXIM     (1u << 4)
#define LCR_FEN       (1u << 4)
#define LCR_STP2      (1u << 3)
#define ICR_RTIC      (1u << 6)
#define ICR_TXIC      (1u << 5)
#define ICR_RXIC      (1u << 4)


struct uart_pl011_regs {
  u32 DR;      /* 0x000 */
  u32 RSR;     /* 0x004 */
  u32 res1[4]; /* 0x008-0x014 */
  u32 FR;      /* 0x018 */
  u32 res2;    /* 0x01C */
  u32 ILPR;    /* 0x020 */
  u32 IBRD;    /* 0x024 */
  u32 FBRD;    /* 0x028 */
  u32 LCR_H;   /* 0x02C */
  u32 CR;      /* 0x030 */
  u32 IFLS;    /* 0x034 */
  u32 IMSC;    /* 0x038 */
  u32 RIS;     /* 0x03C */
  u32 MIS;     /* 0x040 */
  u32 ICR;     /* 0x044 */
  u32 DMACR;   /* 0x048 */
};

struct uart_pl011 {
  u64 base_clock;
  u32 baudrate;
  u16 data_bits;
  u16 stop_bits;
  volatile struct uart_pl011_regs *regs;
};


int uart_init();
char uart_getc(void);
void uart_putc(char c);
void
uart_irq_init(void);
