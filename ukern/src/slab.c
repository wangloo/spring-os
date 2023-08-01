/**
 * @file  slab.c
 * @brief 小块内存分配器
 * @level arch-independent, 向上提供小内存分配的接口
 * @date  2023-07-29
 */
#include <slab.h>
#include <page.h>
#include <kernel.h>


#define MAX_MEM_POOLS       32
#define SLAB_MAGIC			(0xdeadbeef)
#define FREELIST_SET_VAL(freelist, idx, val)  *((int *)(freelist)+idx) = (val)

static struct mem_pool pool_pool = {
    .name = "mempool-pool",
    .obj_size = sizeof(struct mem_pool),
    .size = sizeof(struct mem_pool),
    .gfporder = 0,
};

struct pool_info pool_list[] = {
    {"mempool-8",               8},   {"mempool-16",             16},   
    {"mempool-32",             32},   {"mempool-64",             64},   
    {"mempool-128",           128},   {"mempool-256",           256},  
    {"mempool-512",           512},   {"mempool-1024",         1024},  
};

// index 0: pool of <struct mem_pool>
struct mem_pool *pool_ptrs[MAX_MEM_POOLS];
int nr_pools = 1;



int is_slab_addr(void *addr)
{
    // TODO: 可以划出一块 va 为mempool, 这样所有
    //       从mempool分配的内存都可以定位了
    return 1;
}


static void mp_list_init(struct mem_pool *pool)
{
    INIT_LIST_HEAD(&pool->partial);
    INIT_LIST_HEAD(&pool->full);
}

static void slab_freelist_init(void *freelist, int cnt)
{
    for (int i = 0; i < cnt; i++) {
        FREELIST_SET_VAL(freelist, i, i);
    }
}

// order 实际没有使用, 因为此时每个slab只能占用一个page.
// 原因是page metadata和page的对应方式比较简单
static int caculate_nr_object(struct slab *slab, int order, int obj_size)
{
    int nr_obj;
    size_t freelist_size;
    void *obj_start;
    void *page_end;

    page_end = slab->page + PAGE_SIZE*(1<<order);

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


static void mp_init_boot(void)
{
    // 初始化分配 struct mem_pool 的mempool
    // 链表设置为空即可, 需要时可以refill
    pool_ptrs[0] = &pool_pool;
    mp_list_init(pool_ptrs[0]);
}

struct mem_pool *
mem_pool_create(char *name, size_t obj_size)
{
    struct mem_pool *pool;
    
    // TODO: 检查是否存在 obj_size 重复的 pool

    pool = _mpalloc(pool_ptrs[0]);
    if (!pool)
        assert(0);
    
    pool->name = name;
    pool->obj_size = obj_size;
    pool->size = pool->obj_size;
    pool->gfporder = 0;
    mp_list_init(pool);
    pool_ptrs[nr_pools++] = pool;

    return pool;
}

void mem_pool_destory(struct mem_pool *pool)
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
    mpfree(pool);
}

void mem_pool_init(void)
{
    int pool_static_num;
    int i;

    pool_static_num = sizeof(pool_list) / sizeof(pool_list[0]);
    assert(nr_pools + pool_static_num <= MAX_MEM_POOLS);
    
    mp_init_boot();

    for (i = 0; i < pool_static_num; i++) 
        mem_pool_create(pool_list[i].name, 
                        pool_list[i].size);
}





static struct slab *
slab_alloc(int order)
{
    struct slab *new;
    void *page;

    page = get_free_page(0);
    new = (struct slab *)(page);
    new->page = page;
    new->freelist = page + sizeof(*new);
    return new;
}

// slab 系统的核心函数
static int slab_refill(struct mem_pool *pool)
{
    struct slab *new;
    int nr_obj;
    void *freelist_end;

    LOG_DEBUG("KMEM", "mempool <%s> has been refilled", pool->name);
    // 从系统中索取几页 物理页面
    // page, freelist 成员会被赋值
    new = slab_alloc(pool->gfporder);
    new->magic = SLAB_MAGIC;
    // 初始化object的分布, 这部分是重点
    // 确定这个页面可以存放多少个 object?
    nr_obj = caculate_nr_object(new, pool->gfporder, pool->size);
    freelist_end = new->freelist + nr_obj*sizeof(int);
    new->nr_free = nr_obj;
    new->nr_obj = nr_obj;
    new->obj_start = (void *)align_up((vaddr_t)freelist_end, 8);
    new->pool = pool;
    slab_freelist_init(new->freelist, nr_obj);
    
    list_add(&new->lru, &pool->partial);
    return 0;
}

void *_mpalloc(struct mem_pool *pool)
{
    struct slab *slab;
    void *obj;
    int res, idx;

    assert(pool);

    LOG_INFO("KMEM", "alloc from pool: %s", pool->name);

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
void *mpalloc(int size)
{
    struct mem_pool *pool = NULL;
    int pool_num, i;

    assert(size < PAGE_SIZE);
    // check mem_pool_init() has been called
    assert(pool_ptrs[0] && pool_ptrs[0]->obj_size);

    pool_num = sizeof(pool_list) / sizeof(pool_list[0]);
    
    // TODO: size对齐到8字节

    // 找到合适的 mem pool
    for (i = 0; i < pool_num; i++) {
        if (pool_ptrs[i]->obj_size >= size) {
            pool = pool_ptrs[i];
            break;
        }
    }
    if (!pool) {
        assert(0);
        return NULL;
    }
    return _mpalloc(pool);
}

void mpfree(void *ptr)
{
    struct slab *slab;
    void *page;
    int obj_size;
    int idx;

    // ptr is allocated by mempool
    assert(is_slab_addr(ptr));

    page = (void *)align_page_down((vaddr_t)ptr);
    slab = (struct slab *)page;
    obj_size = slab->pool->obj_size;
    idx = (ptr - slab->obj_start) / obj_size;

    // slab is vaild
    assert(slab->magic == SLAB_MAGIC);
    // ptr in object region of a slab
    assert(ptr >= slab->obj_start);
    // ptr is the start of a object
    assert((ptr - slab->obj_start) % obj_size == 0);

    // 更新 slab
    slab->active -= 1;
    slab->nr_free += 1;
    FREELIST_SET_VAL(slab->freelist, slab->active, idx);
    // 
    if (slab->nr_free == slab->nr_obj) {
        list_del(&slab->lru);
        free_pages(page);
    }

}