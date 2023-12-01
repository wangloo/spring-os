#pragma once
#include <minos/page.h>

/*
 * total 512G address space for process.
 * 256G - 512G for kernel use to map shared memory
 * 255G - 256G stack for process. (8k is reserve).
 * 0 - 4k reserve
 */
#define PROCESS_ADDR_TOP	(1UL << 38)
#define PROCESS_ADDR_BOTTOM	(PAGE_SIZE)

#define PROCESS_STACK_TOP	(PROCESS_ADDR_TOP - PAGE_SIZE)
#define PROCESS_STACK_SIZE	(1UL * 1024 * 1024 * 1024 - PAGE_SIZE)
#define PROCESS_STACK_INIT_SIZE	(8 * PAGE_SIZE)
#define PROCESS_STACK_INIT_BASE	(PROCESS_STACK_TOP - PROCESS_STACK_INIT_SIZE)
#define PROCESS_STACK_BASE	(PROCESS_STACK_TOP - PROCESS_STACK_SIZE)

#define PROCESS_MMAP_TOP	(PROCESS_STACK_BASE - PAGE_SIZE)
#define PROCESS_MMAP_SIZE	(4UL * 1024 * 1024 * 1024 - PAGE_SIZE)
#define PROCESS_MMAP_BOTTOM	(PROCESS_MMAP_TOP - PROCESS_MMAP_SIZE)

#define PROCESS_BRK_TOP		(1UL << 32)