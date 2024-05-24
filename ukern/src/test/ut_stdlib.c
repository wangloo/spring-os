#include <unity.h>
#include <kernel.h>

static char str[32] = {0};


// static void __ut_call
void
test_strcmp(void)
{
  strcpy(str, "hello");
  TEST_ASSERT_EQUAL(0, strcmp(str, "hello"));
  TEST_ASSERT_LESS_THAN(0, strcmp(str, "hellow"));
  TEST_ASSERT_GREATER_THAN(0, strcmp(str, "hel"));
}

void
test_strchr(void)
{
  memset(str, 0, 32);
  strcpy(str, "abcd");
  TEST_ASSERT_EQUAL(str+1, strchr(str, 'b'));
}

void
ut_stdlib(void)
{
  RUN_TEST(test_strcmp);
  printf("%s %d\n", __FILE__, __LINE__);

  RUN_TEST(test_strchr);
}

__define_ut_call(ut_stdlib);