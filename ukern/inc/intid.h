#pragma once

typedef enum {
    INTID_UART_NS = 32 + 0x1,

    //IRQ_HPTIMER = , /* EL2 Hypervisor physical timer */
    //IRQ_SPTIMER = , /* EL3 Secure physical timer */
    INTID_PTIMER = 16 + 0xe, /*  EL1 Non-secure physical timer */
    INTID_VTIMER = 16 + 0xb, /* A virtual timer */
} E_INTID;