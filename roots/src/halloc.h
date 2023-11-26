struct slab_header {
  unsigned long size;
  union {
    unsigned long magic;
    struct slab_header *next;
  };
}__packed;

struct slab_cache {
  unsigned long size;
  struct list_head list;
  struct slab_header *head;
};




void *
halloc_zero(size_t size);
void *
halloc_page(void);
void *
halloc_pagezero(void);
void
hfree(void *addr);
int 
halloc_init(unsigned long base, unsigned long end);