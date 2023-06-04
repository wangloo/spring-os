#include <kmem.h>
#include <assert.h>

void *kalloc(size_t size)
{
  int pages;

  assert(size > 0);

  if (size < PAGE_SIZE) {
    return mpalloc(size);
  }
  
  pages = align_page_up(size);
  return get_free_pages(pages, 0);
}


void kfree(void *addr)
{
  assert(0);
}