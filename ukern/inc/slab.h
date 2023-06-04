#include <list.h>
#include <page.h>

// 一个slab中各字段的分布:
//       | struct slab | freelist | padding | obj1 |
//       | obj2 | obj3 | obj4 | obj5 |     objx    |
// 
//  - 未采用slab的着色机制
//  - struct slab和freelist均放到slab指向的物理页帧中
//  - object后没有添加padding，即没有越界的检查

struct mem_pool {
    char *name;
    int size;     // plus padding
    int obj_size; // without padding
    int gfporder; // NOT USED

    struct list_head partial;
    struct list_head full;
};

struct slab {
    struct list_head lru;
    void *obj_start;    // addr of first object
    void *page;        
    struct mem_pool *pool; // 所属的mem pool
    void *freelist; 
    unsigned long magic;
    int active;
    int nr_free, nr_obj;
};

struct pool_info {
    char *name;
    unsigned long size;
};


    
void *_mpalloc(struct mem_pool *pool);
void *mpalloc(int size);
void mpfree(void *ptr);
void mem_pool_init(void);
struct mem_pool *mem_pool_create(char *name, size_t obj_size);
void mem_pool_destory(struct mem_pool *pool);
int is_slab_addr(void *addr);



// debug
void DBG_mem_pool(struct mem_pool *pool);
void DBG_mem_pools();