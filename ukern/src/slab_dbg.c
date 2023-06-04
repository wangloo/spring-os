#include <print.h>
#include <assert.h>
#include <slab.h>

void print_freelist(int depth, struct slab *slab)
{
  int nr_obj = slab->nr_obj;
  int i;

  print_space(2*depth);
  printf("freelist:");
  for (i = 0; i < nr_obj; i++) {
    if (i % 10 == 0) {
      printf("\n"); 
      print_space(2*depth);
  }
    printf("%d, ", *((int *)slab->freelist+i));
  }
  print_space(2*depth);
  printf("\n");

}

static void print_slab(int depth, struct slab *slab)
{

  print_space(2*depth);
  printf("active: %d\n", slab->active);
  print_space(2*depth);
  printf("nr_free: %d\n", slab->nr_free);
  print_space(2*depth);
  printf("nr_obj: %d\n", slab->nr_obj);
  print_space(2*depth);
  printf("page: (%p, %p)\n", slab->page, 
                             slab->page+0x1000);
  print_space(2*depth);
  printf("freelist: (%p, %p)\n", slab->freelist, 
            slab->freelist + slab->nr_obj*sizeof(int));
  print_space(2*depth);
  printf("obj: %p\n", slab->obj_start);
  print_freelist(depth, slab);
}


static void print_slab_list(int depth, struct slab *slab, struct list_head *head)
{
  int idx = 0;
  assert(slab);

  while (&slab->lru != head) {
    print_space(2*depth);
    printf("slab[%d]:\n", idx);
    print_slab(depth+1, slab);
    slab = list_next_entry(slab, lru);
    idx += 1;
  }
}

static void print_pool_list(int depth, struct mem_pool *pool)
{
  struct slab *slab;

  assert(pool);

  print_space(2*depth);
  printf("partial:\n");
  slab = list_first_entry_or_null(&pool->partial, struct slab, lru);
  if (slab) {
    print_slab_list(depth+1, slab, &pool->partial);
  }
}

void DBG_mem_pool(struct mem_pool *pool)
{
  assert(pool);

  printf("%s(objsize: %d):\n", pool->name, pool->obj_size);

  print_pool_list(1, pool);
}

void DBG_mem_pools()
{
  extern struct mem_pool *pool_ptrs[];
  extern int nr_pools;

  for (int i = 0; i < nr_pools; i++) {
    DBG_mem_pool(pool_ptrs[i]);
  }
}
