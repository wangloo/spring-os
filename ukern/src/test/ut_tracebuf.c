#include <unity.h>
#include <kernel.h>
#include <kmon/tracebuf.h>

static struct tracebuf *ttb;

void
test_alloc_tracebuf(void)
{
  ttb = alloc_tracebuf(64);
  TEST_ASSERT_NOT_EQUAL(NULL, ttb);
  TEST_ASSERT_NOT_EQUAL(NULL, ttb->data);
}

void
test_opt_tracebuf(void)
{
  char s1[64] = {0};
  int i;

  for (i = 0; i < 5; i++) {
    sprintf(s1, "Example%2d\n", i);
    TEST_ASSERT_EQUAL(10+4, tracebuf_insert(ttb, s1));
  }

  // print_tracebuf_info(ttb);
  // printf("Tracebuf Data:\n");
  // print_tracebuf_raw(ttb);
  
  char *ts = NULL;
  int tslen = 0;
  
  tslen = tracebuf_to_str(ttb, &ts);
  TEST_ASSERT_EQUAL(40, tslen); // 1 items are released
  // printf("Out String:\n");
  // printf("len: %d\n", tslen);
  // printf("%s",ts);

}
  
    
void
ut_tracebuf(void)
{
  RUN_TEST(test_alloc_tracebuf);
  RUN_TEST(test_opt_tracebuf);

}

__define_ut_call(ut_tracebuf);