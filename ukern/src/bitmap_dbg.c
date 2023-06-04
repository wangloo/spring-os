#include <assert.h>
#include <print.h>
#include <bitmap.h>

static void DBG_bitmap_print(bitmap_t *bitmap, u64 size)
{
  u64 wsize = size / sizeof(long);
  for (int i = 0; i < wsize; i++) {
    printf("- bitmap[%ld] = 0x%lx\n", i, bitmap[i]);
  }
}
int DBG_bitmap(void)
{
  u64 pos, i;
  bitmap_t bitmap[8];
  
  bitmap_init(bitmap, BITMAP_SIZE(512), BITMAP_EMPTY);
  pos = bitmap_find_first_0(bitmap, BITMAP_SIZE(512));
  assert(pos == 0);

  bitmap_set_bit(bitmap, 0);
  bitmap_set_bit(bitmap, 2);
  pos = bitmap_find_first_0(bitmap, BITMAP_SIZE(512));
  assert(pos == 1);
  
  bitmap_set_bits(bitmap, 0, WORD_BITS);
  pos = bitmap_find_first_0(bitmap, BITMAP_SIZE(512));
  assert(pos == WORD_BITS);
  bitmap_clear_bits(bitmap, 0, WORD_BITS);
  
  // test find_next_0() / find_next_1() / find_next_0_area()
  for (i = WORD_BITS-1; i >= 40; i--) 
    bitmap_set_bit(bitmap, i);
  bitmap_set_bits(bitmap, 0, 10);
  assert(bitmap_find_next_0(bitmap, BITMAP_SIZE(512),  0) == 10);
  assert(bitmap_find_next_0(bitmap, BITMAP_SIZE(512), 20) == 20);
  assert(bitmap_find_next_0(bitmap, BITMAP_SIZE(512), 40) == WORD_BITS);
  assert(bitmap_find_next_1(bitmap, BITMAP_SIZE(512),  0) == 0 );
  assert(bitmap_find_next_1(bitmap, BITMAP_SIZE(512), 20) == 40);
  assert(bitmap_find_next_1(bitmap, BITMAP_SIZE(512), 40) == 40);
  assert(bitmap_find_next_0_area(bitmap, BITMAP_SIZE(512), 0, 20) ==        10);
  assert(bitmap_find_next_0_area(bitmap, BITMAP_SIZE(512), 0, 50) == WORD_BITS);

  DBG_bitmap_print(bitmap, BITMAP_SIZE(512));
  return 0;
}