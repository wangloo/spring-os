#include <unity.h>
#include <kernel.h>
#include <slab.h>

static struct slab_pool *pool;
static void
mysetup(void)
{
  // Test slab with testpool, 
  // for no impact on main slab
  // pool = slab_pool_alloc("TestPool", 16);
  // assert(pool);
}

static void
myteardown(void)
{
  slab_pool_free(pool);
}



static void
UtSlabInit(void)
{
  pool = slab_pool_create("TestPool", 64);
  TEST_ASSERT_NOT_EMPTY(&pool);
  TEST_ASSERT_EQUAL(64, pool->obj_size);
}

static void
UtSlabAlloc(void)
{
  struct slab *slab;
  int count = 64;
  int i, slabs_full=0, slabs_part=0;
  int *nums[count];


  for (i = 0; i < count; i++) {
    nums[i] = slab_alloc_pool(pool);
    TEST_ASSERT_NOT_EMPTY(&nums[i]);
    *(nums[i]) = i+100;
  }
  TEST_ASSERT_EQUAL(163, *(nums[63]));
  TEST_ASSERT_EQUAL(100, *(nums[0]));
  
  // Two slabs are filled in this pool
  list_for_each_entry(slab, &pool->partial, lru) {
    TEST_ASSERT_EQUAL(pool, slab->pool);
    TEST_ASSERT_GREATER_THAN(0, slab->nr_free);
    slabs_part += 1;
  }
  list_for_each_entry(slab, &pool->full, lru) {
    TEST_ASSERT_EQUAL(pool, slab->pool);
    TEST_ASSERT_EQUAL(0, slab->nr_free);
    slabs_full += 1;
  }
  TEST_ASSERT_EQUAL(1, slabs_part);
  TEST_ASSERT_EQUAL(1, slabs_full);

  for (i = 0; i < count; i++) {
    slab_free(nums[i]);
  }
}


void
ut_slab(void)
{
  mysetup();
  RUN_TEST(UtSlabInit);
  RUN_TEST(UtSlabAlloc);
  // RUN_TEST(test_strchr);
  myteardown();
}
__define_ut_call(ut_slab);