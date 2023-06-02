#pragma once
#include <panic.h>

#define assert(condition)                                                      \
  do {                                                                         \
    if (!(condition)) {                                                        \
      panic("ASSERT FAIL: " #condition " (%s line:%d)\n", __func__, __LINE__); \
    }                                                                          \
  } while (0)
