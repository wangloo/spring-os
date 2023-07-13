#include <mm.h>
#include <config/config.h>
#include <assert.h>
#include <kmem.h>
#include <addrspace.h>

void mm_init(void)
{
  extern int kern_vspace_init(void);

  vaddr_t kmem_base;
  size_t kmem_size;

  /* 直接访问会默认使用adr指令， 因为跨越高低两个地址，
     所以adr指令访问不到，这时要借用ldr伪指令来实现 */
  asm volatile(
  "ldr %0, =kernel_end\n"
  "ldr %0, [%0]\n"
  : "=&r"(kmem_base)
  :);

  kmem_base = align_page_up(kmem_base);
  kmem_size = CONFIG_KERNEL_RAM_SIZE;

  assert(IS_PAGE_ALIGN(kmem_base) && IS_PAGE_ALIGN(kmem_size));
  page_section_add_kern(vtop(kmem_base), kmem_size >> PAGE_SHIFT);

  // 2. 初始化内核内存分配器 kalloc接口,
  //    slab 做小内存分配
  mem_pool_init();

  // 3. 映射
  kern_vspace_init();

  // 用户内存怎么办?


  // DBG_mem_pools();
}
