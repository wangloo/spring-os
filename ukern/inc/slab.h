#include <list.h>

// 一个slab中各字段的分布:
//       | struct slab | freelist | padding | obj1 |
//       | obj2 | obj3 | obj4 | obj5 |     objx    |
// 
//  - 未采用slab的着色机制
//  - struct slab和freelist均放到slab指向的物理页帧中
//  - object后没有添加padding，即没有越界的检查

struct slab_cache {
  void *obj;
  struct slab_cache *next;
};
struct slab_pool {
    char *name;
    int size;     // plus padding NOT USED
    int obj_size; // without padding
    int gfporder; // NOT USED

    struct list_head partial;
    struct list_head full;
    struct slab_cache *cache;
};

struct slab {
    void *obj_start;    // addr of first object
    // void *obj_end;      // End addr of last object
    //                     // Not always equal to end of page, might have padding
    // void *page;        
    struct slab_pool *pool; // slab-pool it belongs
    short *freelist; 
    unsigned long magic;
    int active;            // Number of used objs / Index of next freed obj
    int nr_free, nr_obj;
    struct list_head lru;
};

struct pool_info {
    char *name;
    unsigned long size;
};

int 
init_slab(void);
void 
slab_free(void *ptr);
void *
slab_alloc(int bytes);
void *
slab_alloc_pool(struct slab_pool *pool);
struct slab_pool *
slab_pool_create(char *name, int objsize);
void 
slab_pool_free(struct slab_pool *pool);
int 
slab_own_addr(void *ptr);

// debug
// void DBG_mem_pool(struct mem_pool *pool);
// void DBG_mem_pools();
void
print_slab_info(void);