/**
 * @file  mm.c
 * @brief 管理整个内存系统，包括内核分配器用的、用户用的等所有内存。
 * @level 向下需要访问架构相关， 负责初始化各个分配器。
 * @date  2023-07-29
 */

#include <kernel.h>
#include <page.h>
#include <slab.h>
#include <kvspace.h>
#include <mm.h>




// Print variables in image linkscript
void
print_imageinfo(void)
{
  extern unsigned long __get__start_boot_code();
  extern unsigned long __get__stop_boot_code();
  extern unsigned long __get__start_boot_data();
  extern unsigned long __get__kernel_page_table();
  extern unsigned long __get__identity_page_table();
  extern unsigned long __get__pagetable_base();
  extern unsigned long __get__stop_boot_data();
  extern unsigned long __get__start_kernel();
  extern unsigned long __get__start_code();
  extern unsigned long __get__stop_code();
  extern unsigned long __get__kobj_type_desc_start();
  extern unsigned long __get__kobj_type_desc_stop();
  extern unsigned long __get__start_data();
  extern unsigned long __get__start_bss();
  extern unsigned long __get__stop_bss();
  extern unsigned long __get__start_rodata();
  extern unsigned long __get__stop_kernel();
  printf("Symbals in link script:\n");
  printf("\t[__start_boot_code      ]: 0x%lx\n", __get__start_boot_code());
  printf("\t[__stop_boot_code       ]: 0x%lx\n", __get__stop_boot_code());
  printf("\t[__start_boot_data      ]: 0x%lx\n", __get__start_boot_data());
  printf("\t[__kernel_page_table    ]: 0x%lx\n", __get__kernel_page_table());
  printf("\t[__identity_page_table  ]: 0x%lx\n", __get__identity_page_table());
  printf("\t[__pagetable_base       ]: 0x%lx\n", __get__pagetable_base());
  printf("\t[__stop_boot_data       ]: 0x%lx\n", __get__stop_boot_data());
  printf("\t[__start_kernel         ]: 0x%lx\n", __get__start_kernel());
  printf("\t[__start_code           ]: 0x%lx\n", __get__start_code());
  printf("\t[__stop_code            ]: 0x%lx\n", __get__stop_code());
  printf("\t[__kobj_type_desc_start ]: 0x%lx\n", __get__kobj_type_desc_start());
  printf("\t[__kobj_type_desc_stop  ]: 0x%lx\n", __get__kobj_type_desc_stop());
  printf("\t[__start_data           ]: 0x%lx\n", __get__start_data());
  printf("\t[__start_bss            ]: 0x%lx\n", __get__start_bss());
  printf("\t[__stop_bss             ]: 0x%lx\n", __get__stop_bss());
  printf("\t[__start_rodata         ]: 0x%lx\n", __get__start_rodata());
  printf("\t[__stop_kernel          ]: 0x%lx\n", __get__stop_kernel());
}

// Print memory partition done at boot stage
// And also print usage
void
print_meminfo(void)
{
  extern uint64_t deref_kernel_start();
  extern uint64_t deref_kernel_end();
  extern uint64_t deref_kernel_bootmem_base();
  extern uint64_t deref_kernel_stack_bottom();
  extern uint64_t deref_kernel_stack_top();
  extern uint64_t deref_kernel_stack_top();
  extern uint64_t deref_boot_pgtbl_base();
  extern uint64_t deref_boot_pgtbl_ptr();
  printf("Symbals in assembly boot code: \n");
  printf("\t[kernel_start       ]: 0x%lx\n", deref_kernel_start()); 
  printf("\t[kernel_end         ]: 0x%lx\n", deref_kernel_end()); 
  printf("\t[kernel_bootmem_base]: 0x%lx\n", deref_kernel_bootmem_base()); 
  printf("\t[kernel_stack_bottom]: 0x%lx\n", deref_kernel_stack_bottom()); 
  printf("\t[kernel_stack_top   ]: 0x%lx\n", deref_kernel_stack_top()); 
  printf("\t[boot_pgtbl_base    ]: 0x%lx\n", deref_boot_pgtbl_base()); 
  printf("\t[boot_pgtbl_ptr     ]: 0x%lx\n", deref_boot_pgtbl_ptr()); 
}

void 
init_mm(void)
{
  paddr_t kmem_base;
  size_t kmem_size;

  print_imageinfo();
  print_meminfo();

  

  // `kernel_end` is a symbol defined in asm
  // Use inline asm because C will use `adr` to load variable
  // But adr can't load var in lower-addrspace from higher-addrspace
  // So we use pseudo instruction `ldr` here
  asm volatile(
    "ldr %0, =kernel_end\n"
    "ldr %0, [%0]\n"
    : "=&r"(kmem_base)
    :);

  kmem_base = align_page_up(kmem_base);
  kmem_size = CONFIG_RAM_SIZE;
  assert(page_aligned(kmem_base) && page_aligned(kmem_size));

  // System memory allocator init
  // After do this, kalloc() can work
  page_init(kmem_base, kmem_size>>PAGE_SHIFT);
  if (init_slab() < 0) {
    LOG_ERROR("Slab system is bad\n");
    assert(0);
  }
  

  kvspace_init();

  // 用户内存怎么办?

  // Kmem_test();
  // DBG_mem_pools();
}
