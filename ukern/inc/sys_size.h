#pragma once

#define SIZE_1G    (0x40000000UL)
#define SIZE_4K    (0x1000)
#define SIZE_1M    (0x100000)
#define SIZE_1K    (0x400)
#define SIZE_16K   (16 * SIZE_1K)
#define SIZE_32M   (32 * SIZE_1M)
#define SIZE_64K   (64 * SIZE_1K)
#define SIZE_512M  (512 * SIZE_1M)
#define SIZE_2M    (2 * 1024 * 1024)
#define SIZE_8M    (8 * 1024 * 1024)


#define PAGE_SHIFT (12) // 4K
#define PAGE_SIZE  (1 << PAGE_SHIFT)
#define PAGE_MASK  (PAGE_SIZE - 1)