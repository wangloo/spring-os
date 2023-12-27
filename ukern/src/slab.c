#include <kernel.h>
#include <page.h>
#include <slab.h>


#define MAX_MEM_POOLS       32
#define SLAB_MAGIC			(0xdeadbeef)


// Only this pool is staticlly created pool
// Used to allocate memory for other pre-defined slab pools
static struct slab_pool root_pool = {
    .name = "[root-pool]",
    .obj_size = sizeof(struct slab_pool),
    .size = sizeof(struct slab_pool),
    .gfporder = 0,
};

// System pre-defined general pools
static struct pool_info general_pools[] = {
    {"slabpool-8",               8},   {"slabpool-16",             16},   
    {"slabpool-32",             32},   {"slabpool-64",             64},
    {"slabpool-128",           128},   {"slabpool-256",           256},  
    {"slabpool-512",           512},   
};

// index 0 ==> root_pool
static struct slab_pool *pool_ptrs[MAX_MEM_POOLS];
static int nr_pools = 1;



// Free a slab pool
void 
slab_pool_free(struct slab_pool *pool)
{
  int pool_index = -1;
  int i;

  // 确定没有slab挂在上面了
  assert(pool);
  assert(list_empty(&pool->full) && list_empty(&pool->partial));

  for (i = 0; i < nr_pools; i++) {
    if (pool_ptrs[i] == pool) {
      pool_index = i;
      break;
    }
  }
  assert(pool_index > 0); // boot pool can't be destory

  memmove(pool_ptrs + pool_index, pool_ptrs + pool_index + 1,
          nr_pools - pool_index - 1);
  nr_pools -= 1;
  slab_free((void *)pool);
}


// order 实际没有使用, 因为此时每个slab只能占用一个page.
// 原因是page metadata和page的对应方式比较简单
static int 
caculate_nr_object(struct slab *slab, int order, int obj_size)
{
  int nr_obj;
  unsigned long page_end, obj_start;

  page_end = (unsigned long)slab + PAGE_SIZE * (1 << order);

  // 不可能达到的上限肯定是所有的空间都用来存 object
  // 由于此版本只考虑freelist放在slab内部的情况,
  // 所以实际上还是要为freelist留出空间
  nr_obj = (page_end - (unsigned long)(slab->freelist)) / obj_size;

  // 保证满足最大的对齐要求
  obj_start = align_up((vaddr_t)(slab->freelist+nr_obj), obj_size);
  while (obj_start + nr_obj * obj_size > page_end) {
    nr_obj--;
    obj_start = align_up((vaddr_t)(slab->freelist+nr_obj), obj_size);
  }
  assert(nr_obj > 0);
  // printf("caculate result: nrobj: %d \n", nr_obj);
  return nr_obj;
}

// slab 系统的核心函数
static int 
slab_refill(struct slab_pool *pool)
{
  struct slab *slab;
  int nr_obj;

  // LOG_DEBUG("mempool <%s> has been refilled\n", pool->name);

  if (!(slab = page_alloc()))
    return -1;
  slab->freelist = (void *)slab+sizeof(*slab);
  slab->magic = SLAB_MAGIC;

  // 初始化object的分布, 这部分是重点
  // 确定这个页面可以存放多少个 object?
  nr_obj = caculate_nr_object(slab, pool->gfporder, pool->size);
  slab->active = 0;
  slab->nr_free = nr_obj;
  slab->nr_obj = nr_obj;
  slab->obj_start = (void *)align_up((vaddr_t)(slab->freelist+nr_obj), pool->size);
  slab->pool = pool;

  for (int i = 0; i < nr_obj; i++)
    slab->freelist[i] = i;

  list_add(&slab->lru, &pool->partial);
  return 0;
}





// TODO: check is header of slab object
int 
slab_own_addr(void *ptr)
{
  struct slab_pool *sp;
  struct slab *s;
  int i;
  // Go through all pools
  for (i = 1; i < nr_pools; i++) {
    sp = pool_ptrs[i];
    // LOG_DEBUG("go through slab %s\n", sp->name);
    list_for_each_entry(s, &sp->partial, lru) {
      if (ptr >= s->obj_start && 
          ptr < (s->obj_start + sp->obj_size*s->nr_obj)) {
        goto found;
      }
    }
    list_for_each_entry(s, &sp->full, lru) {
      if (ptr >= s->obj_start && 
          ptr < (s->obj_start + sp->obj_size*s->nr_obj)) {
        goto found;
      }
    }
  }
  // LOG_DEBUG("Slab don't own this ptr %d\n", ptr);
  return 0; // Not found
found:
  // LOG_DEBUG("0x%lx is owned by slab pool[%s]\n",ptr, sp->name);
  return 1;
}



void
print_slab_info(void)
{
  struct slab_pool *pool;
  struct slab *slab;
  int i,slab_index;

  for (i = 0; i < nr_pools; i++) {
    pool = pool_ptrs[i];
    slab_index = 0;
    printf("\nPool [%s]:\n", pool->name);
    printf("Partial:\n");
    list_for_each_entry(slab, &pool->partial, lru) {
      assert(slab->magic == SLAB_MAGIC);
      printf("slab%d\nnr_obj: %d\nnr_free:%d\n", slab_index, slab->nr_obj, slab->nr_free);
      slab_index += 1;
    }
  }
  
}


// Free the slab memory to slab-pool
void 
slab_free(void *ptr)
{
  struct slab *slab;
  int obj_index;


  // Size of each slab always == PAGE_SIZE
  // So page_down alignment can get slab desciptor address
  slab = (struct slab *)align_page_down((vaddr_t)ptr);

  // Check the slab descriptor is valid
  assert(slab->magic == SLAB_MAGIC);
  assert(slab->obj_start <= ptr);
  assert(slab->nr_free < slab->nr_obj);
  assert(isaligned(ptr, slab->pool->obj_size));


  // Update slab descriptor
  slab->active--;
  slab->nr_free++;
  obj_index = (ptr - slab->obj_start) / slab->pool->obj_size;
  slab->freelist[slab->active] = (short)obj_index;

  // Return the whole slab to page allocator
  // if all of its objects are free
  if (slab->nr_free == slab->nr_obj) {
    list_del(&slab->lru);
    page_free(slab);
  }
}

void
slab_magic_check(void)
{
  struct slab_pool *pool;
  struct slab *slab;
  for (int i = 0; i < nr_pools; i++) {
    pool = pool_ptrs[i];
    list_for_each_entry(slab, &pool->partial, lru) {
      assert(slab->magic == SLAB_MAGIC);
    }
    list_for_each_entry(slab, &pool->full, lru) {
      assert(slab->magic == SLAB_MAGIC);
    }
  }
}

// Allocate slab from specified slab-pool
// Also global function, make sense when creating
// special slab-pool(eg: from some descriptor)
void *
slab_alloc_pool(struct slab_pool *pool)
{
  struct slab *slab;
  void *obj;
  short obj_index;

  assert(pool);
  // LOG_INFO("alloc from pool: %s\n", pool->name);
realloc:
  slab = NULL;

  // Search partial list first
  if (!list_empty(&pool->partial)) {
    list_for_each_entry(slab, &pool->partial, lru) {
      if (slab->nr_free > 0) break;
    }
    // Should not be here
    // At least one slab in partial list
    if (&slab->lru == &pool->partial)
      assert(0); 
  }

  // Slab pool is full now, refill it
  if (!slab) {
    if (slab_refill(pool) < 0) {
      LOG_ERROR("IMPORTANT! slab refill err\n");
      assert(0);
    }
    goto realloc;
  }

  // 有空闲的object, 现在开始提取, 并维护好剩下的
  obj_index = slab->freelist[slab->active];
  obj = slab->obj_start + pool->size * obj_index;
  slab->active += 1;
  slab->nr_free -= 1;

  if (!slab->nr_free) {
    list_del(&slab->lru);
    list_add(&slab->lru, &pool->full);
  }
  // memset(obj, 0xff,pool->obj_size);
  // slab_magic_check();
  return obj;
}

// Allocate bytes in slab system
// Return pointer to va that kernel can acess
// Return NULL if no more free space
void *
slab_alloc(int bytes)
{
  // Strictly speaking, allocate request more that PAGE_SIZE
  // should be servered by page_alloc()
  assert(bytes < PAGE_SIZE);

  // Just make sure mem_pool_init() has been called
  // before any slab allocate request
  assert(pool_ptrs[0] && pool_ptrs[0]->obj_size);

  // Slab pool is arranged size-descending
  // So the first fit == best fit
  for (int i = 1; i <= nelem(general_pools); i++) {
    if (pool_ptrs[i]->obj_size >= bytes)
      return slab_alloc_pool(pool_ptrs[i]);
  }
  LOG_ERROR("Can't find usable slab pool for %d bytes \n", bytes);
  return NULL;
}

static int
slab_pool_init(struct slab_pool *pool, char *name, int objsize)
{
  if (nr_pools >= MAX_MEM_POOLS) {
    LOG_ERROR("Too many slab pools\n");
    return -1;
  }

  pool->name = name;
  pool->obj_size = objsize;
  pool->size = pool->obj_size;
  pool->gfporder = 0;
  INIT_LIST_HEAD(&pool->partial);
  INIT_LIST_HEAD(&pool->full);

  pool_ptrs[nr_pools++] = pool;
  return 0;
}

struct slab_pool *
slab_pool_create(char *name, int objsize)
{
  struct slab_pool *pool;

  if (!(pool = slab_alloc_pool(&root_pool)))
    return NULL;
  if (slab_pool_init(pool, name, objsize) < 0)
    return NULL;
  return pool;
}

// Init the slab allocator.
// After call this, slab_alloc() can work
int 
init_slab(void)
{
  int nr_static_pool = nelem(general_pools);
  int i;


  // Root pool needs to be initialized first
  // So other pools' descriptor can be allocated
  pool_ptrs[0] = &root_pool;
  INIT_LIST_HEAD(&root_pool.partial);
  INIT_LIST_HEAD(&root_pool.full);


  LOG_DEBUG("Create general pools...\n");
  for (i = 0; i < nr_static_pool; i++) {
    if (!slab_pool_create(general_pools[i].name, general_pools[i].size))
      return -1;
  }

  return 0;
}

