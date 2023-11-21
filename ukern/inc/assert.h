#pragma once

#define assert(condition)                                                      \
  do {                                                                         \
    if (!(condition)) {                                                        \
      panic("ASSERT FAIL: " #condition " (%s:%d)\n", __FILE__, __LINE__); \
    }                                                                          \
  } while (0)

#define TODO()                                                                 \
  do {                                                                         \
    panic("INPLEMENT ME! : (%s:%d)\n", __FILE__, __LINE__);               \
  } while (0)
