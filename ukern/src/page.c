#include <page.h>
#include <assert.h>
#include <string.h>
#include <addrspace.h> // 临时起名
#include <print.h>


// index 0 for kernel
static struct page_section page_sections[6];
static int nr_sections = 1;


static void 
page_section_init(struct page_section *ps, paddr_t base, int pages)
{
  struct page_meta *page;
  paddr_t end;

  assert(IS_PAGE_ALIGN(base));
  assert(pages > 0);

  end = base + (pages << PAGE_SHIFT);
  memset(ps, 0, sizeof(*ps));

  // space reserve 1: bitmap
  ps->bitmap = (bitmap_t *)ptov(base);
  base += BITMAP_SIZE(pages);
  bitmap_init(ps->bitmap, BITMAP_SIZE(pages), BITMAP_EMPTY);

  // space reserve 2: page metadata
  page = (struct page_meta *)ptov(base);
  ps->pages = page;
  base += pages * sizeof(*page);
  memset(page, 0, pages*sizeof(*page));
  
  // other space is free
  // 所以实际管理的页面的数量一定是小于bitmap可以管理的数量的,
  // 这样其实也没关系, 剩下的结构不用就是了
  base = align_page_up(base);
  ps->size = end-base;
  ps->pa_base = base;
  ps->va_base = ptov(base);
}

void page_section_add_kern(paddr_t base, int pages)
{
  printf("kernel page section add %d pages, base :0x%lx\n", pages, base); 
  page_section_init(&page_sections[0], base, pages);
}

static struct page_meta *
alloc_pages_from_section(struct page_section *ps, int pages, int flag)
{
  struct page_meta *page;
  int pos;

  pos = bitmap_find_next_0_area(ps->bitmap, 
          BITMAP_SIZE(ps->size>>PAGE_SHIFT), 0, pages);
  if (pos == -1)
    return NULL;

  bitmap_set_bits(ps->bitmap, pos, pages);
  
  page = ps->pages + pos;
  page->count = pages;
  page->pa = (void *)ps->pa_base + pos*PAGE_SIZE;
  printf("allocate %d pages, base pa: 0x%lx\n", pages, page->pa);
  return page;
}

struct page_meta *alloc_pages(int pages, int flags)
{
  struct page_meta *page = NULL;
  int i = 0;

  for (i = 0; i < nr_sections; i++) {
    page = alloc_pages_from_section(page_sections+i, pages, flags);

    if (page)
      return page;
  }
  return NULL;
}


// DONE
void *get_free_pages(int pages, int flag)
{
  struct page_meta *page = NULL;

  page = alloc_pages(pages, flag);
  if (page)
    return page->pa;
    
  return NULL;
}

void *get_free_page(int flag)
{
  return get_free_pages(1, flag);
}



int free_pages(void *addr)
{
  assert(0);
  return 0;
}