#include <kernel.h>
#include <page.h>
#include <slab.h>
#include <kmem.h>

void *kalloc(size_t size)
{
  int pages;

  // assert(size > 0);

  if (size <= PAGE_SIZE/4) {
    return slab_alloc(size);
  }
  pages = align_page_up(size) >> PAGE_SHIFT;
  return page_allocn(pages);
}

void *
kallocz(size_t size)
{
  void *va = kalloc(size);
  if (!va)
    return NULL;
  memset(va, 0, size);
  return va;
}


void kfree(void *addr)
{
  if (slab_own_addr(addr))
    slab_free(addr);
  else
    page_free(addr);
}