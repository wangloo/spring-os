#include <mm.h>
#include <config/config.h>
#include <assert.h>
#include <kmem.h>
#include <addrspace.h>

void mm_init(void)
{
  extern int kern_vspace_init(void);
  // 1. 管理好物理内存
  paddr_t kmem_base;
  int kmem_size;

  assert(0);
  // kmem_base = align_page_up(ptov(kernel_end));
  // kmem_size = CONFIG_KERNEL_RAM_SIZE - (kmem_base - vtop(kernel_start));
  assert(IS_PAGE_ALIGN(kmem_base) && IS_PAGE_ALIGN(kmem_size));
  page_section_add_kern(kmem_base, kmem_size >> PAGE_SHIFT);

  // 2. 初始化内核内存分配器 kalloc接口,
  //    slab 做小内存分配
  mem_pool_init();

  // 3. 映射
  kern_vspace_init();

  // 用户内存怎么办?


  // DBG_mem_pools();
}
