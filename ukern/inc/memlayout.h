// Physical memory layout and 

// qemu -machine virt is set up like this,
// based on qemu's hw/arm/virt.c:

#pragma once

#define PFLASH_BASE             0x04000000ul
#define PFLASH_SIZE             0x4000000
   
#define GICD_BASE               0x08000000ul
#define GICD_REG_SIZE           0xa0000
   
#define GICR_BASE               (GICD_BASE + GICD_REG_SIZE)
#define GICR_REG_RD_SIZE        0x10000
#define GICR_REG_SGI_SIZE       0x10000
#define GICR_REG_SIZE_PERCPU    (GICR_REG_RD_SIZE + GICR_REG_SGI_SIZE)  
   
#define UART_PL011_BASE         0x09000000ul
#define UART_PL011_REG_SIZE     0x1000
   
#define VIRTIO_BASE             0x0a000000ul
#define VIRTIO_SIZE             0x4000
   
// Logical-defined, a piece of free physical memory
#define RAMDISK_BASE            0x44000000ul
#define RAMDISK_SIZE            0x2000000    // 32M