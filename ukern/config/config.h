#pragma once

// software config
#define CONFIG_KERNEL_ADDR_MASK     0xffff000000000000
#define CONFIG_TASK_STACK_SIZE      8192
#define CONFIG_TASK_STACK_SHIFT     13
#define CONFIG_NR_TASKS             128

#define CONFIG_KERNEL_IRQWORK_IRQ   5

#define NCPU          8  // maximum number of CPUs
#define PROC_PRIO_MAX 8  // maximun priority of user process
#define MAXARG       32  // max exec arguments


// hardware config
#define CONFIG_AARCH64
#define CONFIG_NR_CPUS              4
#define CONFIG_NR_CPUS_CLUSTER0     4
#define CONFIG_KERNEL_ENTRY_ADDR    0x40000000
#define CONFIG_RAM_SIZE             0x4000000  // 64M Map at boot stage, kernel rw directly
#define CONFIG_UART_BASE            0x9000000
#define CONFIG_UART_IO_SIZE         0x1000
#define CONFIG_UART_IRQ             33

#define CONFIG_GICD_BASE            0x08000000
#define CONFIG_GICD_IO_SIZE         0xa0000
#define CONFIG_GICR_BASE            (CONFIG_GICD_BASE + CONFIG_GICD_IO_SIZE)
#define CONFIG_GICR_RD_SIZE         0x10000
#define CONFIG_GICR_SGI_SIZE        0x10000
#define CONFIG_GICR_IO_SIZE_PERCPU  (CONFIG_GICR_RD_SIZE + CONFIG_GICR_SGI_SIZE)
#define CONFIG_GICR_IO_SIZE         (CONFIG_GICR_IO_SIZE_PERCPU * CONFIG_NR_CPUS)

#define CONFIG_RAMDISK_BASE         0x44000000
#define CONFIG_RAMDISK_SIZE         0x02000000 // 32M

#define CONFIG_PFLASH_BASE          (0x04000000)
#define CONFIG_PFLASH_SIZE          (0x4000000)

#define CONFIG_VIRTIO_BASE          (0x0a000000)
#define CONFIG_VIRTIO_SIZE          (0x4000)
#define CONFIG_VIRTIO_IRQ           (32 + 0x10)


// DEBUG OPTION
#define CONFIG_DEBUG_LIST