#include <bitmap.h>


#define PAGE_SHIFT (12) // 4K
#define PAGE_SIZE  (1 << PAGE_SHIFT)
#define PAGE_MASK  (PAGE_SIZE - 1)

#define HUGE_PAGE_SHIFT (21) // 2M
#define HUGE_PAGE_SIZE  (1 << HUGO_PAGE_SHIFT)
#define HUGE_PAGE_MASK  (HUGE_PAGE_SIZE - 1)

#define __GFP_KERNEL       0x00000001
#define __GFP_USER         0x00000002
#define __GFP_GUEST        0x00000004
#define __GFP_DMA          0x00000008
#define __GFP_SHARED       0x00000010
#define __GFP_SLAB         0x00000020
#define __GFP_HUGE         0x00000040
#define __GFP_IO           0x00000080

#define GFP_KERNEL         __GFP_KERNEL
#define GFP_USER           __GFP_USER
#define GFP_GUEST          __GFP_GUEST
#define GFP_DMA            __GFP_DMA
#define GFP_SLAB           __GFP_SLAB
#define GFP_SHARED         __GFP_SHARED
#define GFP_SHARED_IO      (__GFP_SHARED | __GFP_IO)
#define GFP_HUGE           (__GFP_USER | __GFP_HUGE)
#define GFP_HUGE_IO        (__GFP_USER | __GFP_HUGE | __GFP_IO)



#define page_aligned(x)   (!((unsigned long)(x) & (PAGE_SIZE - 1)))
#define align_page_up(x)   align_up(x, PAGE_SIZE)
#define align_page_down(x) align_down(x, PAGE_SIZE)

struct page {
    void *pa;    // Physicall address
    int count;   // Number of continuous pages
    // struct list_head list;
};


struct page_section {
    paddr_t pa_base;
    vaddr_t va_base;
    // size_t  size; 
    int nr_pages; // 实际可以用于分配出去的页面数量
    // int nr_pages; // 该section管理的总页面数, 包含管理数据

    bitmap_t *bitmap;

    // spinlock_t lock;

    // struct list_head page_list;
    struct page *pages;
};

void 
page_init(paddr_t base, int pages);
void *
page_alloc(void);
void *
page_allocz(void);
void *
page_allocn(int count);
void 
page_free(void *ptr);


// ======= Abandoned ================
// void page_section_add_kern(paddr_t base, int pages);
// struct page_meta *alloc_pages(int pages, int flags);
// void *get_free_page(int flag);
// void *get_free_page_q(void);
// void *get_free_pages(int pages, int flag);
// int free_pages(void *addr);