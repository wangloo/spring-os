#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

#include <minos/list.h>
#include <minos/utils.h>
#include <minos/types.h>
#include <minos/page.h>
#include <minos/compiler.h>
#include <halloc.h>


#define SLAB_MIN_DATA_SIZE       16 // slab分配器最小分配16个字节
#define SLAB_MIN_DATA_SIZE_SHIFT 4
#define SLAB_HEADER_SIZE (sizeof(struct slab_header))
#define SLAB_MAGIC			(0xdeadbeef)
#define HASH_TABLE_SIZE    8 // hash表最大碰撞次数

static struct list_head slab_hash_table[HASH_TABLE_SIZE];

static void *slab_base;
static void *slab_end;
static uint32_t slab_size;

#define hash(size) (((size) >> SLAB_MIN_DATA_SIZE_SHIFT) % HASH_TABLE_SIZE)


static void *
halloc_from_hash_table(size_t size)
{
  int idx = hash(size);
  struct slab_header *sh;
  struct slab_cache *sc;

  list_for_each_entry(sc, &slab_hash_table[idx], list) {
    if (sc->size != size) 
      continue;

    // TODO: Delete slab_cache if no slab?
    if (!sc->head)
      return NULL; 

    // Pick it from list
    sh = sc->head;
    sc->head = sh->next;
    sh->magic = SLAB_MAGIC;
    
    return ((void *)sh + SLAB_HEADER_SIZE);
  }
  return NULL;
}

static void *
halloc_from_slab_heap(size_t size)
{
  struct slab_header *sh;

  size += SLAB_HEADER_SIZE;
  if (slab_size < size) 
    return NULL;

  sh = (struct slab_header *)slab_base;
  sh->magic = SLAB_MAGIC;
  sh->size = size - SLAB_HEADER_SIZE;

  slab_base += size;
  slab_size -= size;
  return ((void *)sh + SLAB_HEADER_SIZE);
}


static void *__halloc(size_t size)
{
	void *mem;

	mem = halloc_from_hash_table(size);
	if (mem == NULL)
		mem = halloc_from_slab_heap(size);
	if (!mem)
		printf("malloc fail for 0x%lx\n", size);

	return mem;
}

// Heap allocate
void *
halloc(size_t size)
{
	if (size == 0)
		size = SLAB_MIN_DATA_SIZE;
	else
    size = align_up(size, SLAB_MIN_DATA_SIZE);

	return __halloc(size);
}

static void
free_slab(void *addr)
{
  struct slab_header *sh;
  struct slab_cache *sc;
  int idx;
  
  sh = (struct slab_header *)((unsigned long)addr - SLAB_HEADER_SIZE);
  if (sh->magic != SLAB_MAGIC || sh->size < SLAB_MIN_DATA_SIZE) {
    assert(0);
    return;
  }
    
  idx = hash(sh->size);
  list_for_each_entry(sc, &slab_hash_table[idx], list) {
    if (sc->size != sh->size)
      continue;
    
    // Insert into slab_cache
    sh->next = sc->head;
    sc->head = sh;
    return;
  }

  // Create new slab cache
  sc = halloc_from_slab_heap(sizeof(*sc));
  if (sc == NULL) {
    assert(0);
    return;
  }

  sc->size = sh->size;
  sc->head = sh;
  sh->next = NULL;
  list_add_tail(&sc->list, &slab_hash_table[idx]);
}

void
hfree(void *addr)
{
  free_slab(addr);
}

void *
halloc_zero(size_t size)
{
	void *addr = halloc(size);
	if (addr)
		memset(addr, 0, size);
	return addr;
}

void *
halloc_page(void)
{
  return halloc(PAGE_SIZE);
}

void *
halloc_pagezero(void)
{
  return halloc_zero(PAGE_SIZE);
}

int 
halloc_init(unsigned long base, unsigned long end)
{
  int i;
  if ((end < base) || !page_aligned(base) || !page_aligned(end)) {
    fprintf(stderr, "invalid heap region 0x%lx 0x%lx\n", base, end);
    return -EINVAL;
  }

  slab_base = (void *)base;
  slab_end = (void *)end;
  slab_size = end-base;

  for (i = 0; i < HASH_TABLE_SIZE; i++) {
    INIT_LIST_HEAD(&slab_hash_table[i]);
  }
  return 0;
}