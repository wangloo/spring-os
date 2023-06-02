#pragma once

// software config
#define CONFIG_TASK_STACK_SIZE      8192
#define CONFIG_TASK_STACK_SHIFT     13



// hardware config
#define CONFIG_KERNEL_ENTRY_ADDR    0x40000000
#define CONFIG_RAM_SIZE             0x10000000 // 256M
#define CONFIG_UART_BASE            0x9000000
#define CONFIG_UART_IO_SIZE         0x1000
#define CONFIG_UART_IRQ             33
#define CONFIG_NR_CPUS              4