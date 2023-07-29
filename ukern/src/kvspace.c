/**
 * @file  kvspace.c
 * @brief 内核地址空间的操作，
 * @level 底层依赖页表的操作，本文件不涉及任何架构相关操作
 * @date  2023-07-29
 */

#include <vspace.h>
#include <page.h>
#include <memattr.h>
#include <kernel.h>

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

int kern_map_create(vaddr_t va, paddr_t pa,
      size_t size, int flags)
{
  int ret;

	if (!IS_PAGE_ALIGN(va) || !IS_PAGE_ALIGN(pa) ||
			!IS_PAGE_ALIGN(size))
	  return -EINVAL;

  // spin_lock(&host_vspace.lock);
  ret = pagetable_map(kvspace.pgdp, va, pa, size, flags, get_free_page_q);
  // spin_unlock(&host_vspace.lock);
  return ret;
}

int kern_map_destory(vaddr_t va, size_t size)
{
  TODO();
  return 0;
}


/* int kern_map_remap(...); */

int kern_map_const_regions(void)
{
  int i;
  int ret;
  struct map_region *region;

  
  for (i = 0; i < NR_KERN_CONST_MAP; i++) {
    region = kern_const_map + i;
    printf("kvspace: start mapping const region[%d], va: 0x%lx, pa: 0x%lx, size: %lx\n",
            i, region->vbase, region->pbase, region->size);
    ret = kern_map_create(region->vbase, 
                          region->pbase,
                          region->size,  
                          region->flag);
    if (ret) 
      printf("kvspace: failed\n", i);
    else 
      printf("kvspace: ok\n");    
  }
  return 0; 
}

int kern_vspace_init(vaddr_t pgtable_base)
{
  kvspace.pgdp = (page_table_t *)pgtable_base;
  atomic_set(1, &(kvspace.refcount));

  kern_map_const_regions();
  return 0;
}
