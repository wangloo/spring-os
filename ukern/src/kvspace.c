/**
 * @file  kvspace.c
 * @brief 内核地址空间的操作，
 * @level 底层依赖页表的操作，本文件不涉及任何架构相关操作
 * @date  2023-07-29
 */
#include <kernel.h>
#include <atomic.h>
#include <spinlock.h>
#include <page.h>
#include <memattr.h>
#include <pagetable.h>
#include <vspace.h>
#include <kvspace.h>

struct map_region 
{
  vaddr_t vbase;
  paddr_t pbase;
  size_t size;
  int flag;
};

enum _kern_const_map_index 
{
  KERN_CONST_MAP_GICD = 0,
  KERN_CONST_MAP_GICR,
  KERN_CONST_MAP_UART,
  KERN_CONST_MAP_RAMDISK,
  KERN_CONST_MAP_PFLASH,
  KERN_CONST_MAP_VIRTIO,
  NR_KERN_CONST_MAP,
};

struct map_region kern_const_map[] = {
 [KERN_CONST_MAP_GICD] = {
   .pbase = CONFIG_GICD_BASE, 
   .vbase = ptov(CONFIG_GICD_BASE),
   .size  = CONFIG_GICD_IO_SIZE,
   .flag  = VM_IO | VM_RW | VM_HUGE,
 },
 [KERN_CONST_MAP_GICR] = {
   .pbase = CONFIG_GICR_BASE,
   .vbase = ptov(CONFIG_GICR_BASE),
   .size  = CONFIG_GICR_IO_SIZE,
   .flag  = VM_IO | VM_RW | VM_HUGE,
 },
 [KERN_CONST_MAP_UART] = {
  .pbase = CONFIG_UART_BASE,
  .vbase = ptov(CONFIG_UART_BASE),
  .size  = CONFIG_UART_IO_SIZE,
  .flag  = VM_IO | VM_RW ,
 },
 [KERN_CONST_MAP_RAMDISK] = {
  .pbase = CONFIG_RAMDISK_BASE,
  .vbase = ptov(CONFIG_RAMDISK_BASE),
  .size  = CONFIG_RAMDISK_SIZE,
  .flag  = VM_RW | VM_HUGE,
 },
  [KERN_CONST_MAP_PFLASH] = {
    .pbase = CONFIG_PFLASH_BASE,
    .vbase = ptov(CONFIG_PFLASH_BASE),
    .size  = CONFIG_PFLASH_SIZE,
    .flag  = VM_IO | VM_RW | VM_HUGE,
  },
  [KERN_CONST_MAP_VIRTIO] = {
    .pbase = CONFIG_VIRTIO_BASE,
    .vbase = ptov(CONFIG_VIRTIO_BASE),
    .size  = CONFIG_VIRTIO_SIZE,
    .flag  = VM_IO | VM_RW ,
  },
};

static struct vspace kvspace;

static int 
kern_map(vaddr_t va, paddr_t pa, size_t size, int flags)
{
  int ret;

  if (!page_aligned(va) || !page_aligned(pa) || !page_aligned(size)) {
	  return -EINVAL;
  }

  // spin_lock(&host_vspace.lock);
  ret = pagetable_map(kvspace.pgdp, va, pa, size, flags);
  // spin_unlock(&host_vspace.lock);
  return ret;
}

static int 
kern_unmap(vaddr_t va, size_t size)
{
  TODO();
  return 0;
}

static int
kern_remap(vaddr_t old, vaddr_t new, size_t size)
{
  TODO();
  return 0;
}

static void 
kern_map_const_regions(void)
{
  struct map_region *r;
  int ret;

  for (int i = 0; i < NR_KERN_CONST_MAP; i++) {
    r = kern_const_map + i;
    LOG_INFO("start mapping const region[%d], va: 0x%lx, pa: 0x%lx, size: %lx\n",
            i, r->vbase, r->pbase, r->size);

    ret = kern_map(r->vbase, r->pbase, r->size,  r->flag);
    if (ret) {
      LOG_ERROR("failed when mapping region[%d]", i);
    } else {
      LOG_DEBUG("ok\n");    
    }
  }
}


// Return the virtual addr of kernel pagetable
// kerenl pgd is defined in bootdata, mapped in boot stage
pagetable_t 
kern_pagetbl_base(void)
{
  pagetable_t base;

  asm volatile (
  "ldr %0, =__kernel_page_table\n"
  :"=r"(base)
  );

  return ptov(base);
}

// Initialize kernel vspace mapping
// Some address mappings are done in boot stage
// For now, only const regions need to be mapped here
void 
kvspace_init(void)
{
  kvspace.pgdp = (struct pagetable *)kern_pagetbl_base();
  atomic_set(&kvspace.refcount, 1);

  kern_map_const_regions();

//   DBG_pagetable(kvspace.pgdp);
}
