#pragma once

#define assert(condition)                                                      \
  do {                                                                         \
    if (!(condition)) {                                                        \
      panic("ASSERT FAIL: " #condition " (%s line:%d)\n", __func__, __LINE__); \
    }                                                                          \
  } while (0)

#define TODO()                                                                 \
  do {                                                                         \
    panic("INPLEMENT ME! : (%s line:%d)\n", __func__, __LINE__);               \
  } while (0)
