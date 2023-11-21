
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