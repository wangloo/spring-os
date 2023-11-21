#include <kernel.h>
#include <page.h>
#include <slab.h>
#include <kmem.h>

void *kalloc(size_t size)
{
  int pages;

  assert(size > 0);

  if (size < PAGE_SIZE) {
    return slab_alloc(size);
  }
  pages = align_page_up(size) >> PAGE_SHIFT;
  return page_allocn(pages);
}


void kfree(void *addr)
{
  TODO();
}