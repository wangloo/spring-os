#pragma once
#include <vspace.h>
#include <uspace/proc.h>

/*
 * 512G for user space process
 */
#define USER_PROCESS_ADDR_LIMIT		(1UL << 39)

#define PROCESS_TOP_HALF_BASE	(USER_PROCESS_ADDR_LIMIT >> 1)
#define VMA_SHARED_BASE		PROCESS_TOP_HALF_BASE

#define MIN_ELF_LOAD_BASE	0x1000
#define ROOTSRV_USTACK_TOP	(PROCESS_TOP_HALF_BASE - PAGE_SIZE * 8)
#define ROOTSRV_USTACK_BOTTOM	(ROOTSRV_USTACK_TOP - ROOTSRV_USTACK_PAGES * PAGE_SIZE)
#define ROOTSRV_BOOTDATA_BASE	(PROCESS_TOP_HALF_BASE - PAGE_SIZE * 2)

#define pa2sva(phy)		((phy) + VMA_SHARED_BASE)
#define va2sva(va)		(pa2sva(vtop(va)))
#define va2pa(va)		vtop(va)
#define pa2va(pa)		ptov(pa)

/*
 * 0    - (256G - 1) user space memory region
 * 256G - (512G - 1) shared memory mapping space.
 *
 * 0    - ( 64G - 1) ELF (text data bss and other)
 * 64G -> (64G + 256M) heap area
 * 65G  - (255G - 1) VMAP area
 * ((256G - 32K - 4K) -> (256G - 4K)) stack
 *
 * system process can handle it heap by itself, the heap
 * region for system process if 64G --- 64G + 256M
 * kernel will handle the page fault for this heap region.
 */
#define SYS_PROC_HEAP_BASE	(64UL * 1024 * 1024 * 1024)
#define SYS_PROC_HEAP_SIZE	(256UL * 1024 * 1024)
#define SYS_PROC_HEAP_END	(SYS_PROC_HEAP_BASE + SYS_PROC_HEAP_SIZE)

#define SYS_PROC_VMAP_BASE	(65UL * 1024 * 1024 * 1024)
#define SYS_PROC_VMAP_END	(255UL * 1024 * 1024 * 1024)


int user_vspace_init(struct process *proc);
void user_vspace_deinit(struct process *proc);

int user_map_create(struct process *proc, unsigned long vaddr,
		       size_t size, unsigned long phy, unsigned long flags);