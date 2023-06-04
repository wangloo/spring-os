#pragma once
#include <types.h>
#include <bitmap.h>

#define IS_PAGE_ALIGN(x)   (!((unsigned long)(x) & (PAGE_SIZE - 1)))
#define align_page_up(x)   align_up(x, PAGE_SIZE)
#define align_page_down(x) align_down(x, PAGE_SIZE)

struct page_meta {
    void *pa;
    // struct list_head list;
    int count;   // not used
};


struct page_section {
    paddr_t pa_base;
    vaddr_t va_base;
    size_t  size; // 实际可以用于分配出去的size
    int nr_pages; // 该section管理的总页面数, 包含管理数据

    bitmap_t *bitmap;

    // spinlock_t lock;

    // struct list_head page_list;
    struct page_meta *pages;
};



void page_section_add_kern(paddr_t base, int pages);
struct page_meta *alloc_pages(int pages, int flags);
void *get_free_page(int flag);
void *get_free_pages(int pages, int flag);
int free_pages(void *addr);