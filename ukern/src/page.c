/**
 * @file  page.c
 * @brief 对page的管理，页面分配器，由mm.c初始化池子
 * @level arch-independent, 向上提供申请page的接口
 * @date  2023-07-29
 */
#include <kernel.h>
#include <page.h>

// 将page section分割开将来可能有利于按照flag从不同的
// section来分配page, 目前暂时没有使用
static struct page_section page_sections[6];

static void 
page_section_init(struct page_section *ps, paddr_t base, int pages)
{
  struct page *page;
  paddr_t origin = base;

  assert(page_aligned(base));
  assert(pages > 0);

  // space reserve 1: bitmap
  ps->bitmap = (bitmap_t *)ptov(base);
  bitmap_init(ps->bitmap, BITMAP_SIZE(pages), BITMAP_EMPTY);
  base += BITMAP_SIZE(pages);

  // space reserve 2: page metadata
  page = (struct page *)ptov(base);
  ps->pages = page;
  base += pages * sizeof(*page);
  
  // other space is free
  // 所以实际管理的页面的数量一定是小于bitmap可以管理的数量的,
  // 这样其实也没关系, 剩下的结构不用就是了
  base = align_page_up(base);
  ps->nr_pages = pages - ((base-origin) >> PAGE_SHIFT);
  ps->pa_base = base;
  ps->va_base = ptov(base);
}

// Initialize page allocator
// base is physical address
void 
page_init(paddr_t base, int pages)
{
  LOG_INFO("kernel page section add %d pages, base :0x%lx\n", pages, base); 
  page_section_init(&page_sections[0], base, pages);
}

static struct page *
__page_allocn_sect(struct page_section *ps, int count)
{
  int pos;

  if (-1 == (pos = bitmap_find_next_0_area(ps->bitmap, BITMAP_SIZE(ps->nr_pages), 0, count))) {
    return NULL;
  }
  bitmap_set_bits(ps->bitmap, pos, count);
  
  struct page *page = ps->pages + pos;
  page->count = count;
  page->pa = (void *)ps->pa_base + pos*PAGE_SIZE;

  LOG_DEBUG("allocate %d page(s), base pa: 0x%lx\n", count, page->pa);
  return page;
}

// Also global function
// Sometimes caller needs page description 
// of allocated pages
struct page *
__page_allocn(int pages)
{
  struct page *page = NULL;

  for (int i = 0; i < nelem(page_sections); i++) {
    if ((page = __page_allocn_sect(page_sections+i, pages))) {
        return page;
    }
  }

  panic("no more pages in all section!\n");
  return NULL;
}


// Like page_alloc(), but can alloc n pages
// The pages are continuous physically
void *
page_allocn(int count)
{
  struct page *page = __page_allocn(count);

  if (page)
    return (void *)ptov((paddr_t)page->pa);
  
  return NULL;
}

void *
page_allocnz(int count)
{
    void *page = page_allocn(count);

    if (page)
        memset(page, 0, PAGE_SIZE * count);
    return page;
};

// Allocate one 4096-byte page of physical memory.
// Returns a pointer to page's virtual addr that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
page_alloc(void)
{
  return page_allocn(1);
}

// Allocate one 4096-byte page and zero it
void *
page_allocz(void)
{
    return page_allocnz(1);
}


void 
page_free(void *ptr)
{
  struct page_section *ps;
  struct page *p;
  int i, pos=0;

  assert(page_aligned(ptr));
  
  for (i = 0; i < 1; i++) {
    ps = page_sections+i;
    while ((pos = bitmap_find_next_1(ps->bitmap, BITMAP_SIZE(ps->nr_pages), pos)) != -1) {
      p = ps->pages+pos;
      // LOG_DEBUG("pa: 0x%lx\n", p->pa);

      if (ptr >= ptov(p->pa) && ptr < (ptov(p->pa)+p->count*PAGE_SIZE)) {
        bitmap_clear_bit(ps->bitmap, pos);
        memset(p, 0, sizeof(*p));
        return;
      }

      pos += 1;
    } // while
  } // for

  LOG_ERROR("Page section of 0x%lx is not found\n", ptr);
}
