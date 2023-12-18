#include <kernel.h>
#include <page.h>
#include <slab.h>


#define MAX_MEM_POOLS       32
#define SLAB_MAGIC			(0xdeadbeef)

#define freelist_update(freelist, idx, val)  *((int *)(freelist)+idx) = (val)

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
    {"slabpool-512",           512},   {"slabpool-1024",         1024},  
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
    assert(list_empty(&pool->full) && 
            list_empty(&pool->partial));

    for (i = 0; i < nr_pools; i++) {
        if (pool_ptrs[i] == pool) {
            pool_index = i;
            break;
        }
    }
    assert(pool_index > 0); // boot pool can't be destory

    memmove(pool_ptrs+pool_index, 
            pool_ptrs+pool_index+1, 
            nr_pools-pool_index-1);
    nr_pools -= 1;
    slab_free((void *)pool);
}

// Allocate a slab pool
struct slab_pool *
slab_pool_alloc(char *name, size_t obj_size)
{
  struct slab_pool *pool;

  if (nr_pools >= MAX_MEM_POOLS) {
    LOG_ERROR("Too many slab pools\n");
    return NULL;
  }

  // if (nr_pools < nelem(general_pools)) {
  //   LOG_ERROR("Slab system should be init first\n");
  //   return NULL;
  // }

  if ((pool = slab_alloc_pool(pool_ptrs[0])) == NULL)
    return NULL;

  pool->name = name;
  pool->obj_size = obj_size;
  pool->size = pool->obj_size;
  pool->gfporder = 0;
  INIT_LIST_HEAD(&pool->partial);
  INIT_LIST_HEAD(&pool->full);

  pool_ptrs[nr_pools++] = pool;
  return pool;
}


// Init the slab allocator.
// After call this, slab_alloc() can work
void 
slab_init(void)
{
    int nr_static_pool = nelem(general_pools);
    int i;

    assert(nr_pools + nr_static_pool <= MAX_MEM_POOLS);
    
    // Root pool needs to be initialized first
    // So other pools' descriptor can be allocated
    pool_ptrs[0] = &root_pool;
    INIT_LIST_HEAD(&pool_ptrs[0]->partial);
    INIT_LIST_HEAD(&pool_ptrs[0]->full);

    for (i = 0; i < nr_static_pool; i++) {
      slab_pool_alloc(general_pools[i].name, general_pools[i].size);
    }
}


static int 
mem_in_slab(void *addr)
{
    // TODO: 可以划出一块 va 为mempool, 这样所有
    //       从mempool分配的内存都可以定位了
    return 1;
}


// order 实际没有使用, 因为此时每个slab只能占用一个page.
// 原因是page metadata和page的对应方式比较简单
static int 
caculate_nr_object(struct slab *slab, int order, int obj_size)
{
    int nr_obj;
    size_t freelist_size;
    void *obj_start;
    void *page_end;

    page_end = (void *)((unsigned long)slab + PAGE_SIZE*(1<<order));

    // 不可能达到的上限肯定是所有的空间都用来存 object
    // 由于此版本只考虑freelist放在slab内部的情况, 
    // 所以实际上还是要为freelist留出空间
    nr_obj = (page_end - slab->freelist) / obj_size;
    freelist_size = sizeof(int) * nr_obj;

    // 保证满足最大的对齐要求 
    obj_start = (void *)align_up((vaddr_t)(slab->freelist)+freelist_size, 8);
    while (obj_start+nr_obj*obj_size > page_end) {
        nr_obj--;
        freelist_size -= sizeof(int);
        obj_start = (void *)align_up((vaddr_t)(slab->freelist)+freelist_size, 8);
    }
    assert(nr_obj > 0);
    return nr_obj;
}

// Allocate a descriptor of slab
static struct slab *
slab_desc_alloc()
{
    struct slab *new;
    void *page;

    page = page_alloc();
    new = (struct slab *)(page);
    // new->page = page;
    new->freelist = page + sizeof(*new);
    return new;
}

// slab 系统的核心函数
static int slab_refill(struct slab_pool *pool)
{
    struct slab *new;
    int nr_obj;
    void *freelist_end;

    // LOG_DEBUG("mempool <%s> has been refilled\n", pool->name);
    // 从系统中索取几页 物理页面
    // page, freelist 成员会被赋值
    new = slab_desc_alloc(pool->gfporder);
    new->magic = SLAB_MAGIC;
    // 初始化object的分布, 这部分是重点
    // 确定这个页面可以存放多少个 object?
    nr_obj = caculate_nr_object(new, pool->gfporder, pool->size);
    freelist_end = new->freelist + nr_obj*sizeof(int);
    new->nr_free = nr_obj;
    new->nr_obj = nr_obj;
    new->obj_start = (void *)align_up((vaddr_t)freelist_end, 8);
    new->pool = pool;
    
    for (int i = 0; i < nr_obj; i++) {
        freelist_update(new->freelist, i, i);
    }
    
    list_add(&new->lru, &pool->partial);
    return 0;
}

// Allocate slab from specified slab-pool
// Also global function, make sense when creating
// special slab-pool(eg: from some descriptor)
void *
slab_alloc_pool(struct slab_pool *pool)
{
    struct slab *slab;
    void *obj;
    int res, idx;

    assert(pool);

    // LOG_INFO("alloc from pool: %s\n", pool->name);

realloc:
    // 找到有空闲object的slab,
    // 先编译 partial, 再遍历 free
    slab = NULL;
    if (!list_empty(&pool->partial)) {
        list_for_each_entry(slab, &pool->partial, lru) {
            if (slab->nr_free > 0) 
                break;
        }
    }

    // 此mempool中没有空闲的object了
    if (slab == NULL) {
        res = slab_refill(pool);
        if (res)
            assert(0);
        goto realloc;
    }

    // 有空闲的object, 现在开始提取, 并维护好剩下的
    idx = *((int *)(slab->freelist)+slab->active);
    obj = slab->obj_start + pool->size*idx;
    slab->active += 1;
    slab->nr_free -= 1;

    // 该slab所处的链表位置可能已经改变
    if (slab->nr_free == 0) {
        // move to full list
        list_del(&slab->lru);
        list_add(&slab->lru, &pool->full);
    }
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
        if (pool_ptrs[i]->obj_size >= bytes) {
            return slab_alloc_pool(pool_ptrs[i]);
            break;
        }
    }
    LOG_ERROR("Can't find usable slab pool for %d bytes \n", bytes);

    return NULL;
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

// Free the slab memory to slab-pool
void 
slab_free(void *ptr)
{
    struct slab *slab;

    // Check ptr is allocated from slab-pool
    assert(mem_in_slab(ptr));

    // Size of each slab always == PAGE_SIZE
    // So page_down alignment can get slab desciptor address
    slab = (struct slab *)align_page_down((vaddr_t)ptr);

    // Check the slab descriptor is valid
    assert(slab->magic == SLAB_MAGIC);
    assert(slab->obj_start <= ptr);


    // Update slab descriptor
    slab->active --;
    slab->nr_free ++;
    int idx = (ptr - slab->obj_start) / slab->pool->obj_size;
    freelist_update(slab->freelist, slab->active, idx);
    
    // Return the whole slab to page allocator 
    // if all of its objects are free
    if (slab->nr_free == slab->nr_obj) {
        list_del(&slab->lru);
        page_free(slab);
    }
}