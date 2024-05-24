#include <unity.h>
#include <kernel.h>

void
setUp(void)
{
  // LOG_INFO("Unity Test Begin...\n");
}

void
tearDown(void)
{
}




void
unittest(void)
{
  unsigned long ut_call_start = (unsigned long)&__start_ut_call;
  unsigned long ut_call_end = (unsigned long)&__stop_ut_call_used;
  int ut_call_count, i;
  ut_call *fn;

  UNITY_BEGIN();

  ut_call_count = (ut_call_end-ut_call_start) / sizeof(*fn);
  LOG_DEBUG("count: %d\n", ut_call_count);
  fn = (ut_call *)ut_call_start;
  for (i = 0; i < ut_call_count; i++) {
    (*fn)();
    fn++;
  }

  UNITY_END();

  LOG_INFO("Unity Test End, system exit!\n");
  exit();
}